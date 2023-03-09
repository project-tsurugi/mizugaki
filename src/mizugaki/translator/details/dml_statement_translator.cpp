#include "dml_statement_translator.h"

#include <vector>
#include <optional>

#include <takatori/scalar/variable_reference.h>

#include <takatori/relation/graph.h>
#include <takatori/relation/emit.h>
#include <takatori/relation/write.h>
#include <takatori/relation/project.h>

#include <takatori/statement/write.h>

#include <takatori/util/assertion.h>
#include <takatori/util/downcast.h>
#include <takatori/util/fail.h>
#include <takatori/util/optional_ptr.h>
#include <takatori/util/string_builder.h>

#include <yugawara/binding/factory.h>

#include <mizugaki/translator/util/statement_dispatch.h>

#include "relation_info.h"
#include "scalar_expression_translator.h"
#include "relation_expression_translator.h"

namespace mizugaki::translator::details {

using translator_type = shakujo_translator::impl;
using result_type = shakujo_translator_result;
using code_type = shakujo_translator_code;

using ::takatori::util::fail;
using ::takatori::util::optional_ptr;
using ::takatori::util::string_builder;
using ::takatori::util::unsafe_downcast;

namespace relation = ::takatori::relation;
namespace dml = ::shakujo::model::statement::dml;

namespace {

class engine {
public:
    explicit engine(translator_type& translator) noexcept :
            translator_ { translator }
    {}

    [[noreturn]] result_type operator()(::shakujo::model::statement::Statement const& node) const {
        ::takatori::util::fail(string_builder {}
                << "must be a DML statement: "
                << node.kind()
                << string_builder::to_string);
    }

    result_type operator()(dml::EmitStatement const& node) {
        relation_expression_translator e { translator_ };
        auto graph = new_graph();
        relation_info relation;
        auto r = e.process(*node.source(), *graph, relation);
        if (!r) return {};

        using ::takatori::relation::emit;
        auto columns = new_vector<emit::column>(relation.count_variables());
        relation.enumerate_variables([&](::takatori::descriptor::variable const& variable, std::string_view name) {
            columns.emplace_back(variable, name);
        });

        auto&& w = graph->emplace<emit>(std::move(columns));
        w.region() = translator_.region(node.region());
        w.input().connect_to(*r);

        return { std::move(graph) };
    }

    result_type operator()(dml::InsertRelationStatement const& node) {
        relation_expression_translator e { translator_ };
        auto graph = new_graph();
        relation_info source;
        auto r = e.process(*node.source(), *graph, source);
        if (!r) return {};

        auto&& index = find_table(node.table());
        if (!index) return {};

        auto&& table = index->table();
        std::size_t ncolumns = node.columns().empty() ? table.columns().size() : node.columns().size();

        if (source.count_variables() != ncolumns) {
            return report(code_type::inconsistent_columns, node, string_builder {}
                    << ncolumns << " column(s) are expected, but "
                    << "there are " << source.count_variables() << " column(s)");
        }
        std::vector<::takatori::descriptor::variable> values;
        values.reserve(source.count_variables());
        source.enumerate_variables([&](::takatori::descriptor::variable const& variable, std::string_view) {
            values.emplace_back(variable);
        });

        using ::takatori::relation::write;
        auto columns = new_vector<write::column>(ncolumns);
        ::tsl::hopscotch_map<::yugawara::storage::column const*, std::size_t> column_map;
        column_map.reserve(ncolumns);

        if (node.columns().empty()) {
            for (auto&& column : table.columns()) {
                auto pos = columns.size();
                BOOST_ASSERT(pos < values.size()); // NOLINT
                columns.emplace_back(std::move(values[pos]), factory()(column));
                column_map.emplace(std::addressof(column), pos);
            }
        } else {
            relation_info relation { *index };
            for (auto const* column : node.columns()) {
                auto&& c = find_column(relation, *column);
                if (!c) return {};

                auto pos = columns.size();
                BOOST_ASSERT(pos < values.size()); // NOLINT
                columns.emplace_back(std::move(values[pos]), factory()(*c));
                column_map.emplace(c.get(), pos);
            }
        }

        auto keys = new_vector<write::key>(index->keys().size());
        for (auto&& key : index->keys()) {
            auto&& column = key.column();
            if (auto it = column_map.find(std::addressof(column)); it != column_map.end()) {
                keys.emplace_back(columns[it.value()]);
            }
            // NOTE: key columns may not be covered
        }

        auto&& w = graph->emplace<write>(
                ::takatori::relation::write_kind::insert,
                factory()(*index),
                std::move(keys),
                std::move(columns));
        w.region() = translator_.region(node.region());

        w.input().connect_to(*r);

        return { std::move(graph) };
    }

    result_type operator()(dml::UpdateStatement const& node) {
        if (!node.initialize().empty()) {
            return report(code_type::unsupported_statement, node, string_builder {}
                    << "UpdateStatement::initialize must be empty");
        }

        relation_expression_translator e { translator_ };
        auto graph = new_graph();
        relation_info relation;
        auto r = e.process(*node.source(), *graph, relation);
        if (!r) return {};

        auto index = check_table(relation, node.table());
        if (!index) return {};

        using ::takatori::relation::write;
        auto keys = build_keys(relation);
        auto columns = new_vector<write::column>(node.columns().size());
        std::unique_ptr<::takatori::relation::project> projection {};
        for (auto&& mapping : node.columns()) {
            auto c = find_column(relation, *mapping->name());
            if (!c) return {};
            auto v = build_value(node, *mapping->value(), relation, projection);
            if (!v) return {};
            columns.emplace_back(std::move(*v), factory()(*c));
        }

        auto&& w = graph->emplace<write>(
                ::takatori::relation::write_kind::update,
                factory()(std::move(index)),
                std::move(keys),
                std::move(columns));
        w.region() = translator_.region(node.region());

        if (projection) {
            auto&& p = graph->insert(std::move(projection));
            r->connect_to(p.input());
            p.output().connect_to(w.input());
        } else {
            r->connect_to(w.input());
        }
        return { std::move(graph) };
    }

    result_type operator()(dml::DeleteStatement const& node) {
        relation_expression_translator e { translator_ };
        auto graph = new_graph();
        relation_info relation;
        auto r = e.process(*node.source(), *graph, relation);
        if (!r) return {};

        auto index = check_table(relation, node.table());
        if (!index) return {};

        using ::takatori::relation::write;
        auto keys = build_keys(relation);

        auto&& w = graph->emplace<write>(
                ::takatori::relation::write_kind::delete_,
                factory()(*index),
                std::move(keys),
                new_vector<write::column>());
        w.region() = translator_.region(node.region());
        w.input().connect_to(*r);

        return { std::move(graph) };
    }

    result_type operator()(dml::InsertValuesStatement const& node) {
        if (!node.initialize().empty()) {
            return report(code_type::unsupported_statement, node, string_builder {}
                    << "InsertValuesStatement::initialize must be empty");
        }

        auto insert_type = convert(node.conflict_action());

        auto&& index = find_table(node.table());
        if (!index) return {};

        using ::takatori::statement::write;

        auto columns = collect_insert_columns(index, node);
        if (columns.size() != node.columns().size()) {
            return {};
        }
        write::tuple tuple {};
        tuple.elements().reserve(node.columns().size());
        for (auto&& column : node.columns()) {
            scalar_expression_translator t { translator_ };
            auto v = t.process(*column->value(), { translator_.options(), {} });
            if (!v) return {};

            tuple.elements().push_back(std::move(v));
        }

        std::vector<write::tuple> tuples {};
        tuples.emplace_back(std::move(tuple));

        auto result = create<write>(
                insert_type,
                factory()(*index),
                std::move(columns),
                std::move(tuples));
        result->region() = translator_.region(node.region());

        return { std::move(result) };
    }

private:
    translator_type& translator_;

    result_type report(code_type code, ::shakujo::model::Node const& node, string_builder&& builder) {
        translator_.diagnostics().emplace_back(
                code,
                builder << string_builder::to_string,
                translator_.region(node.region()));
        return {};
    }

    [[nodiscard]] std::unique_ptr<relation::graph_type> new_graph() const {
        return create<relation::graph_type>();
    }

    template<class T>
    [[nodiscard]] std::vector<T> new_vector(std::size_t capacity = 0) const {
        std::vector<T> result {};
        result.reserve(capacity);
        return result;
    }

    template<class T, class... Args>
    [[nodiscard]] std::unique_ptr<T> create(Args&&... args) const {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

    [[nodiscard]] ::yugawara::binding::factory factory() const noexcept {
        return {};
    }

    [[nodiscard]] std::shared_ptr<::yugawara::storage::index const> find_table(::shakujo::model::name::Name const* name) {
        auto index = translator_.find_table(*name);
        if (!index) {
            report(code_type::table_not_found, *name, string_builder {}
                    << "table `" << *name << "' is not found");
            return {};
        }
        return index;
    }

    [[nodiscard]] std::shared_ptr<::yugawara::storage::index const> check_table(relation_info const& relation, ::shakujo::model::name::Name const* name) {
        auto index = find_table(name);
        if (!index) return {};

        if (!relation.table_index()) {
            report(code_type::inconsistent_table, *name, string_builder {}
                    << "source relation has no valid table info, "
                    << "expected: " << index->table().simple_name());
            return {};
        }
        auto&& table = relation.table_index()->table();
        if (table != index->table()) {
            report(code_type::inconsistent_table, *name, string_builder {}
                    << "source table: " << table.simple_name() << ", "
                    << "expected: " << index->table().simple_name());
            return {};
        }
        return index;
    }

    [[nodiscard]] optional_ptr<::yugawara::storage::column const> find_column(
            relation_info& relation,
            ::shakujo::model::name::SimpleName const& name) {
        if (auto c = relation.find_table_column(name)) {
            return c;
        }
        report(code_type::column_not_found, name, string_builder {}
                << name);
        return {};
    }

    [[nodiscard]] std::vector<::takatori::relation::write::key> build_keys(relation_info const& relation) const {
        auto index = relation.table_index();
        BOOST_ASSERT(index); // NOLINT
        auto keys = new_vector<::takatori::relation::write::key>(index->keys().size());
        for (auto&& key : index->keys()) {
            auto v = relation.find_variable(key.column());
            BOOST_ASSERT(v); // NOLINT
            keys.emplace_back(*v, factory()(key.column()));
        }
        return keys;
    }

    [[nodiscard]] std::optional<::takatori::descriptor::variable> build_value(
            ::shakujo::model::Node const& parent,
            ::shakujo::model::expression::Expression const& node,
            relation_info const& relation,
            std::unique_ptr<::takatori::relation::project>& projection) const {
        scalar_expression_translator scalars { translator_ };
        auto v = scalars.process(node, { translator_.options(), relation });
        if (!v) return {};

        // return variable directly if it is just a variable reference
        if (v->kind() == ::takatori::scalar::variable_reference::tag) {
            auto&& vref = unsafe_downcast<::takatori::scalar::variable_reference>(*v);
            return std::move(vref.variable());
        }

        // otherwise, creates a new projection column and returns it
        if (!projection) {
            using ::takatori::relation::project;
            projection = std::make_unique<project>(new_vector<project::column>());
            projection->region() = translator_.region(parent.region());
        }
        auto result = factory().stream_variable();
        result.region() = v->region();
        projection->columns().emplace_back(result, std::move(v));
        return result;
    }

    [[nodiscard]] static ::takatori::statement::write_kind convert(
            ::shakujo::model::statement::dml::InsertValuesStatement::ConflictAction action) noexcept {
        using from = ::shakujo::model::statement::dml::InsertValuesStatement::ConflictAction;
        using to = ::takatori::statement::write_kind;
        switch (action) {
        case from::ERROR: return to::insert;
        case from::SKIP: return to::insert_skip;
        case from::REPLACE: return to::insert_overwrite;
        }
        fail();
    }

    std::vector<::takatori::statement::write::column> collect_insert_columns(
            std::shared_ptr<::yugawara::storage::index const> const& index,
            dml::InsertValuesStatement const& node) {
        if (node.columns().empty()) {
            return {};
        }
        std::vector<::takatori::statement::write::column> results {};
        results.reserve(node.columns().size());
        if (node.columns()[0]->name() != nullptr) {
            // explicit results
            relation_info table { *index };
            for (auto&& column : node.columns()) {
                if (column->name() == nullptr) {
                    report(code_type::inconsistent_columns, *node.table(), string_builder {}
                            << "unmatched inserting value");
                    return {};
                }
                if (column->value() == nullptr) {
                    report(code_type::inconsistent_columns, *node.table(), string_builder {}
                            << "too short number of inserting values");
                    return {};
                }
                auto c = table.find_table_column(*column->name());
                if (!c) return {};
                results.emplace_back(factory()(*c));
            }
        } else {
            // implicit results
            for (auto&& col : index->table().columns()) {
                if (!col.features().contains(::yugawara::storage::column_feature::hidden)) {
                    results.emplace_back(factory()(col));
                }
            }
            if (node.columns().size() < results.size()) {
                report(code_type::inconsistent_columns, *node.table(), string_builder {}
                        << "too short number of inserting values");
                return {};
            }
            if (node.columns().size() > results.size()) {
                report(code_type::inconsistent_columns, *node.columns()[results.size()]->value(), string_builder {}
                        << "unmatched inserting value");
                return {};
            }
        }
        return results;
    }
};

} // namespace

dml_statement_translator::dml_statement_translator(translator_type& translator) noexcept
    : translator_(translator)
{}

result_type dml_statement_translator::process(::shakujo::model::statement::Statement const& node) {
    engine e { translator_ };
    return util::dispatch(e, node);
}

bool dml_statement_translator::is_supported(shakujo::model::statement::Statement const& node) {
    using k = shakujo::model::statement::StatementKind;
    switch (node.kind()) {
        case k::EMIT_STATEMENT:
        case k::INSERT_VALUES_STATEMENT:
        case k::INSERT_RELATION_STATEMENT:
        case k::UPDATE_STATEMENT:
        case k::DELETE_STATEMENT:
            return true;
        default:
            return false;
    }
    std::abort();
}

} // namespace mizugaki::translator::details
