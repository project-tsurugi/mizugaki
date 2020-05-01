#include "relation_expression_translator.h"

#include <variant>
#include <vector>

#include <takatori/descriptor/variable.h>

#include <takatori/scalar/variable_reference.h>

#include <takatori/relation/scan.h>
#include <takatori/relation/filter.h>
#include <takatori/relation/project.h>
#include <takatori/relation/intermediate/join.h>
#include <takatori/relation/intermediate/distinct.h>
#include <takatori/relation/intermediate/limit.h>
#include <takatori/relation/intermediate/aggregate.h>

#include <takatori/util/assertion.h>
#include <takatori/util/downcast.h>
#include <takatori/util/fail.h>
#include <takatori/util/object_creator.h>
#include <takatori/util/optional_ptr.h>
#include <takatori/util/string_builder.h>

#include <yugawara/binding/factory.h>

#include <mizugaki/translator/util/expression_dispatch.h>

#include "scalar_expression_translator.h"
#include "aggregate_analyzer.h"

namespace mizugaki::translator::details {

using translator_type = shakujo_translator::impl;
using result_type = ::takatori::util::optional_ptr<::takatori::relation::expression::output_port_type>;
using diagnostic_type = shakujo_translator_diagnostic;
using code_type = diagnostic_type::code_type;

using ::takatori::util::fail;
using ::takatori::util::optional_ptr;
using ::takatori::util::string_builder;
using ::takatori::util::unique_object_ptr;
using ::takatori::util::unsafe_downcast;

namespace descriptor = ::takatori::descriptor;
namespace scalar = ::takatori::scalar;
namespace relation = ::takatori::relation;

template<class T>
using object_vector = std::vector<T, ::takatori::util::object_allocator<T>>;

namespace {

template<class T>
class transpose {
public:
    constexpr transpose(T value) noexcept : value_(value) {} // NOLINT
    constexpr T operator*() const noexcept { return value_; }

private:
    T value_;
};

class engine {
public:
    explicit engine(
            translator_type& translator,
            ::takatori::relation::graph_type& graph,
            relation_info& relation) noexcept
        : translator_(translator)
        , graph_(graph)
        , relation_(relation)
    {}

    [[nodiscard]] result_type process(::shakujo::model::expression::Expression const& node) {
        return util::dispatch(*this, node);
    }

    result_type operator()(::shakujo::model::expression::Expression const& node) {
        return report(code_type::unsupported_relational_operator, node, string_builder {}
                << "must be a relation expression: "
                << node.kind());
    }

    result_type operator()(::shakujo::model::expression::relation::ScanExpression const& node) {
        auto c = translator_.object_creator();
        auto&& index = translator_.context().find_table(*node.table());
        if (!index) {
            return report(code_type::table_not_found, *node.table(), string_builder {}
                    << *node.table());
        }

        relation_ = relation_info { *index, *node.table(), c };

        auto columns = new_vector<relation::scan::column>(relation_.count_variables());
        for (auto&& column : index->table().columns()) {
            auto v = relation_.find_variable(column);
            BOOST_ASSERT(v); // NOLINT
            columns.emplace_back(factory()(column), *v);
        }

        using endpoint = relation::scan::endpoint;
        auto&& r = create<relation::scan>(
                node,
                factory()(*index),
                std::move(columns),
                endpoint { c },
                endpoint { c },
                std::nullopt,
                c);
        resolve(r);
        return r.output();
    }

    result_type operator()(::shakujo::model::expression::relation::RenameExpression const& node) {
        auto source = process(*node.operand());
        if (!source) return {};

        relation_.rename_relation(*node.name());
        if (!node.columns().empty()) {
            relation_.rename_columns(node.columns());
        }
        return source;
    }

    result_type operator()(::shakujo::model::expression::relation::SelectionExpression const& node) {
        auto source = process(*node.operand());
        if (!source) return {};

        scalar_expression_translator e { translator_ };
        auto c = e.process(*node.condition(), { translator_.context(), relation_ });
        if (!c) return {};

        auto&& r = create<relation::filter>(
                node,
                std::move(c),
                translator_.object_creator());
        source->connect_to(r.input());
        resolve(r);
        return r.output();
    }

    result_type operator()(::shakujo::model::expression::relation::ProjectionExpression const& node) {
        result_type source {};
        optional_ptr<::shakujo::model::expression::relation::GroupExpression const> group {};
        if (node.operand()->kind() == ::shakujo::model::expression::relation::GroupExpression::tag) {
            group = unsafe_downcast<::shakujo::model::expression::relation::GroupExpression>(*node.operand());
            source = process(*group->operand());
        } else {
            source = process(*node.operand());
        }
        if (!source) return {};

        bool saw_aggregate = false;
        relation_info next;
        auto columns = new_vector<relation::project::column>(node.columns().size());
        for (auto const* column : node.columns()) {
            scalar_expression_translator e { translator_ };
            auto v = e.process(*column->value(), { translator_.context(), relation_ });
            if (!v) return {};

            if (!saw_aggregate) {
                saw_aggregate = e.saw_aggregate();
            }

            auto name = column_name(*column);
            // FIXME: register the column into current variable scope

            if (auto vv = extract_variable(*v)) {
                next.add_column(*vv, std::move(name));
            } else {
                auto&& decl = columns.emplace_back(factory().stream_variable(name), std::move(v));
                auto&& a = translator_.expression_analyzer();
                a.resolve(decl.value(), false, translator_.types());
                a.variables().bind(decl.variable(), decl.value());
                if (column->alias() != nullptr) {
                    decl.variable().region() = translator_.region(column->alias()->region());
                } else {
                    decl.variable().region() = translator_.region(column->value()->region());
                }
                next.add_column(decl.variable(), std::move(name));
            }
        }
        if (columns.empty() && !group) {
            // may not occur
            BOOST_ASSERT(!saw_aggregate); // NOLINT
            relation_ = std::move(next);
            return source;
        }

        auto&& r = create<relation::project>(
                node,
                std::move(columns),
                translator_.object_creator());
        source->connect_to(r.input());
        if (group || saw_aggregate) {
            return process_aggregate(std::move(next), r, group);
        }
        relation_ = std::move(next);
        return r.output();
    }
    
    result_type operator()(::shakujo::model::expression::relation::JoinExpression const& node) {
        relation_info relation_left;
        auto output_left = branch(*node.left(), relation_left);
        if (!output_left) return {};

        relation_info relation_right;
        auto output_right = branch(*node.right(), relation_right);
        if (!output_right) return {};

        relation_ = relation_info { std::move(relation_left), std::move(relation_right) };

        unique_object_ptr<scalar::expression> cond {};
        if (node.condition() != nullptr) {
            scalar_expression_translator e { translator_ };
            cond = e.process(*node.condition(), { translator_.context(), relation_ });
            if (!cond) return {};
        }

        auto ks = convert(node.operator_kind());
        if (std::holds_alternative<relation::join_kind>(ks)) {
            auto k = std::get<relation::join_kind>(ks);
            auto&& r = create<relation::intermediate::join>(
                    node,
                    k,
                    std::move(cond),
                    translator_.object_creator());
            output_left->connect_to(r.left());
            output_right->connect_to(r.right());
            resolve(r);
            return r.output();
        }
        if (std::holds_alternative<transpose<relation::join_kind>>(ks)) {
            auto k = std::get<transpose<relation::join_kind>>(ks);
            auto&& r = create<relation::intermediate::join>(
                    node,
                    *k,
                    std::move(cond),
                    translator_.object_creator());
            // transpose inputs
            output_right->connect_to(r.left());
            output_left->connect_to(r.right());
            resolve(r);
            return r.output();
        }
        return report(code_type::unsupported_relational_operator, node, string_builder {}
                << "unsupported join kind: "
                << node.operator_kind());
    }

    result_type operator()(::shakujo::model::expression::relation::DistinctExpression const& node) {
        auto source = process(*node.operand());
        if (!source) return {};

        auto keys = new_vector<descriptor::variable>(relation_.count_variables());
        relation_.enumerate_variables([&](descriptor::variable const& variable, std::string_view) {
            keys.emplace_back(variable);
        });

        auto&& r = create<relation::intermediate::distinct>(
                node,
                std::move(keys),
                translator_.object_creator());
        source->connect_to(r.input());
        resolve(r);
        return r.output();
    }

    result_type operator()(::shakujo::model::expression::relation::OrderExpression const& node) {
        auto source = process(*node.operand());
        if (!source) return {};

        auto keys = new_vector<relation::intermediate::limit::sort_key>(node.elements().size());
        for (auto&& elem : node.elements()) {
            scalar_expression_translator e { translator_ };
            auto v = e.process(*elem->key(), { translator_.context(), relation_ });
            if (!v) return {};

            auto vv = extract_variable(*v);
            if (!vv) {
                return report(code_type::unsupported_relational_operator, *elem->key(), string_builder {}
                        << "order key must be a plain variable");
            }

            auto dir = convert(elem->direction());
            keys.emplace_back(*vv, dir);
        }

        auto&& r = create<relation::intermediate::limit>(
                node,
                std::nullopt,
                new_vector<descriptor::variable>(),
                keys,
                translator_.object_creator());
        source->connect_to(r.input());
        resolve(r);
        return r.output();
    }

    result_type operator()(::shakujo::model::expression::relation::LimitExpression const& node) {
        auto source = process(*node.operand());
        if (!source) return {};

        // combine if Order -> Limit
        if (source->owner().kind() == relation::intermediate::limit::tag) {
            auto&& limit = unsafe_downcast<relation::intermediate::limit>(source->owner());
            limit.count(node.count());
            return source;
        }

        auto&& r = create<relation::intermediate::limit>(
                node,
                node.count(),
                new_vector<descriptor::variable>(),
                new_vector<relation::intermediate::limit::sort_key>(),
                translator_.object_creator());
        source->connect_to(r.input());
        resolve(r);
        return r.output();
    }

private:
    translator_type& translator_;
    ::takatori::relation::graph_type& graph_;
    relation_info& relation_;

    result_type report(code_type code, ::shakujo::model::Node const& node, string_builder&& builder) {
        translator_.diagnostics().emplace_back(
                code,
                builder << string_builder::to_string,
                translator_.region(node.region()));
        return {};
    }

    template<class T, class... Args>
    T& create(::shakujo::model::Node const& node, Args&&... args) {
        auto&& result = graph_.emplace<T>(std::forward<Args>(args)...);
        result.region() = translator_.region(node.region());
        return result;
    }

    void resolve(relation::expression const& expr) {
        translator_.expression_analyzer().resolve(expr, false, true, translator_.types());
    }

    [[nodiscard]] ::yugawara::binding::factory factory() const noexcept {
        return ::yugawara::binding::factory { translator_.object_creator() };
    }

    template<class T>
    [[nodiscard]] object_vector<T> new_vector(std::size_t capacity = 0) const {
        object_vector<T> result { translator_.object_creator().allocator() };
        if (capacity > 0) {
            result.reserve(capacity);
        }
        return result;
    }

    static optional_ptr<descriptor::variable> extract_variable(scalar::expression& expr) {
        if (expr.kind() == scalar::variable_reference::tag) {
            return unsafe_downcast<scalar::variable_reference>(expr).variable();
        }
        return {};
    }

    static std::string column_name(::shakujo::model::expression::relation::ProjectionExpression::Column const& column) {
        if (column.alias() != nullptr) {
            return column.alias()->token();
        }
        if (column.value()->kind() == ::shakujo::model::expression::VariableReference::tag) {
            auto&& ref = unsafe_downcast<::shakujo::model::expression::VariableReference>(*column.value());
            if (ref.name()->kind() == ::shakujo::model::name::SimpleName::tag) {
                return unsafe_downcast<::shakujo::model::name::SimpleName>(*ref.name()).token();
            }
        }
        return {};
    }

    [[nodiscard]] result_type process_aggregate(
            relation_info next,
            relation::project& projection,
            optional_ptr<::shakujo::model::expression::relation::GroupExpression const> group) {
        auto group_keys = new_vector<descriptor::variable>();
        if (group) {
            group_keys.reserve(group->keys().size());
            for (auto&& key : group->keys()) {
                scalar_expression_translator e { translator_ };
                auto v = e.process(*key, { translator_.context(), relation_ });
                if (!v) return {};
                auto vv = extract_variable(*v);
                if (!vv) {
                    return report(code_type::unsupported_relational_operator, *key, string_builder {}
                            << "group key must be a plain variable");
                }
                group_keys.emplace_back(std::move(*vv));
            }
        }
        auto&& aggregation = graph_.emplace<relation::intermediate::aggregate>(
                std::move(group_keys),
                new_vector<relation::intermediate::aggregate::column>(),
                translator_.object_creator());
        aggregation.region() = projection.region();
        aggregation.input().reconnect_to(*projection.input().opposite());

        aggregate_analyzer analyzer { translator_ };
        if (!analyzer.process(projection, aggregation, relation_)) return {};
        if (!analyzer.validate_group_member(next)) return {};

        relation_ = std::move(next);
        if (projection.columns().empty()) {
            // no columns in the original projection
            graph_.erase(projection);
            return aggregation.output();
        }
        projection.input().connect_to(aggregation.output());
        return projection.output();
    }

    [[nodiscard]] static relation::sort_direction convert(
            ::shakujo::model::expression::relation::OrderExpression::Direction direction) noexcept {
        using from = decltype(direction);
        using to = relation::sort_direction;
        switch (direction) {
            case from::ASCENDANT: return to::ascendant;
            case from::DESCENDANT: return to::descendant;
        }
        fail();
    }

    [[nodiscard]] static std::variant<
            std::monostate,
            relation::join_kind,
            transpose<relation::join_kind>>
    convert(::shakujo::model::expression::relation::JoinExpression::Kind kind) noexcept {
        using from = decltype(kind);
        using to = relation::join_kind;
        switch (kind) {
            case from::CROSS: return to::inner;
            case from::INNER: return to::inner;
            case from::LEFT_OUTER: return to::left_outer;
            case from::RIGHT_OUTER: return transpose { to::left_outer };
            case from::FULL_OUTER: return to::full_outer;
            default: return {};
        }
        fail();
    }

    [[nodiscard]] result_type branch(
            ::shakujo::model::expression::Expression const& node,
            relation_info& new_relation) {
        engine e { translator_, graph_, new_relation };
        return e.process(node);
    }
};

} // namespace

relation_expression_translator::relation_expression_translator(translator_type& translator) noexcept
    : translator_(translator)
{}

relation_expression_translator::result_type relation_expression_translator::process(
        ::shakujo::model::expression::Expression const& node,
        ::takatori::relation::graph_type& graph,
        relation_info& relation) {
    engine e { translator_, graph, relation };
    return e.process(node);
}

} // namespace mizugaki::translator::details
