#include "aggregate_analyzer.h"

#include <algorithm>
#include <vector>

#include <takatori/scalar/dispatch.h>

#include <takatori/util/assertion.h>
#include <takatori/util/downcast.h>
#include <takatori/util/ownership_reference.h>
#include <takatori/util/string_builder.h>

#include <yugawara/binding/factory.h>

#include <yugawara/extension/scalar/aggregate_function_call.h>

namespace mizugaki::translator::details {

using translator_type = shakujo_translator::impl;
using diagnostic_type = shakujo_translator_diagnostic;
using code_type = diagnostic_type::code_type;

namespace descriptor = ::takatori::descriptor;
namespace scalar = ::takatori::scalar;
namespace relation = ::takatori::relation;

using ::takatori::util::downcast;
using ::takatori::util::optional_ptr;
using ::takatori::util::string_builder;
using ::takatori::util::unique_object_ptr;
using ::takatori::util::unsafe_downcast;
using ::takatori::util::object_ownership_reference;

using ::yugawara::extension::scalar::aggregate_function_call;

template<class T>
using object_vector = std::vector<T, ::takatori::util::object_allocator<T>>;

namespace {

class engine {
public:
    explicit engine(
            translator_type& translator,
            ::tsl::hopscotch_set<::takatori::descriptor::variable>& group_members,
            relation::intermediate::aggregate& aggregation)
        : translator_(translator)
        , group_members_(group_members)
        , aggregation_(aggregation)
    {}

    [[nodiscard]] bool process(relation::project::column& column) {
        if (process(column.value())) {
            consume(column.release_value(), std::move(column.variable()));
        }
        return !saw_error_;
    }

    bool operator()(scalar::expression const& expr) {
        translator_.diagnostics().emplace_back(
                code_type::unsupported_scalar_expression,
                string_builder {}
                        << "unknown expression while analyzing aggregate operations: "
                        << expr.kind()
                        << string_builder::to_string,
                expr.region());
        saw_error_ = true;
        return false;
    }

    bool operator()(scalar::immediate const&) noexcept {
        return false;
    }

    bool operator()(scalar::variable_reference const& expr) {
        if (group_members_.find(expr.variable()) != group_members_.end()) {
            translator_.diagnostics().emplace_back(
                    code_type::invalid_aggregation_column,
                    "column must be aggregated",
                    expr.region());
            saw_error_ = true;
        }
        return false;
    }

    bool operator()(scalar::cast& expr) {
        if (process(expr.operand())) {
            consume(expr.ownership_operand());
        }
        return false;
    }

    bool operator()(scalar::unary& expr) {
        if (process(expr.operand())) {
            consume(expr.ownership_operand());
        }
        return false;
    }

    bool operator()(scalar::binary& expr) {
        if (process(expr.left())) {
            consume(expr.ownership_left());
        }
        if (process(expr.right())) {
            consume(expr.ownership_right());
        }
        return false;
    }

    bool operator()(scalar::compare& expr) {
        if (process(expr.left())) {
            consume(expr.ownership_left());
        }
        if (process(expr.right())) {
            consume(expr.ownership_right());
        }
        return false;
    }

    bool operator()(scalar::match& expr) {
        if (process(expr.input())) {
            consume(expr.ownership_input());
        }
        if (process(expr.pattern())) {
            consume(expr.ownership_pattern());
        }
        if (process(expr.escape())) {
            consume(expr.ownership_escape());
        }
        return false;
    }

    bool operator()(scalar::function_call& expr) {
        auto&& args = expr.arguments();
        for (auto it = args.begin(); it != args.end(); ++it) {
            if (process(*it)) {
                consume(args.ownership(it));
            }
        }
        return false;
    }

    bool operator()(scalar::extension& expr) {
        using ::yugawara::extension::scalar::extension_id;
        if (expr.extension_id() == extension_id::aggregate_function_call_id) {
            return operator()(unsafe_downcast<::yugawara::extension::scalar::aggregate_function_call>(expr));
        }
        translator_.diagnostics().emplace_back(
                code_type::unsupported_scalar_expression,
                string_builder {}
                        << "unknown scalar expression extension: "
                        << expr.extension_id()
                        << string_builder::to_string,
                expr.region());
        saw_error_ = true;
        return false;
    }

    bool operator()(aggregate_function_call const&) noexcept {
        // FIXME: check nested aggregations
        return true;
    }

private:
    translator_type& translator_;
    ::tsl::hopscotch_set<::takatori::descriptor::variable>& group_members_;
    relation::intermediate::aggregate& aggregation_;
    optional_ptr<relation::project> arguments_ {};
    bool saw_error_ {};

    [[nodiscard]] bool process(scalar::expression& expr) {
        if (saw_error_) {
            return false;
        }
        return scalar::dispatch(*this, expr);
    }

    void resolve(descriptor::variable const& variable, descriptor::aggregate_function const& desc) {
        auto&& e = translator_.expression_analyzer();
        auto&& info = ::yugawara::binding::unwrap(desc);
        e.variables().bind(variable, info.declaration(), true);
    }

    void consume(object_ownership_reference<scalar::expression> expr) {
        auto variable = bindings().stream_variable();
        variable.region() = expr->region();
        auto vref = translator_.object_creator().create_unique<scalar::variable_reference>(variable);
        vref->region() = expr->region();
        auto aggregate = expr.exchange(std::move(vref));
        consume(std::move(aggregate), std::move(variable));
    }

    void consume(unique_object_ptr<scalar::expression> expr, descriptor::variable variable) {
        BOOST_ASSERT(downcast<aggregate_function_call>(expr.get()) != nullptr); // NOLINT

        auto&& a = unsafe_downcast<aggregate_function_call>(*expr);
        resolve(variable, a.function());

        object_vector<descriptor::variable> arguments { translator_.object_creator().allocator() };
        arguments.reserve(a.arguments().size());
        while (!a.arguments().empty()) {
            auto e = a.arguments().release_back();
            arguments.emplace_back(capture(std::move(e)));
        }
        std::reverse(arguments.begin(), arguments.end());
        aggregation_.columns().emplace_back(std::move(a.function()), std::move(arguments), std::move(variable));

        expr.reset();
    }

    [[nodiscard]] ::yugawara::binding::factory bindings() const noexcept {
        return ::yugawara::binding::factory { translator_.object_creator() };
    }

    [[nodiscard]] descriptor::variable capture(unique_object_ptr<scalar::expression> expr) {
        if (auto v = extract_variable(*expr)) {
            return *v;
        }
        if (!arguments_) {
            BOOST_ASSERT(!aggregation_.is_orphan()); // NOLINT
            arguments_ = aggregation_.owner().emplace<relation::project>(
                    object_vector<relation::project::column> { translator_.object_creator().allocator() },
                    translator_.object_creator());
            auto origin = aggregation_.input().reconnect_to(arguments_->output());
            BOOST_ASSERT(origin); // NOLINT
            arguments_->input().reconnect_to(*origin);
        }
        auto v = bindings().stream_variable();
        v.region() = expr->region();
        auto&& column = arguments_->columns().emplace_back(std::move(expr), v);

        auto&& a = translator_.expression_analyzer();
        a.resolve(column.value(), false, translator_.types());
        a.variables().bind(column.variable(), column.value());
        return v;
    }

    [[nodiscard]] static optional_ptr<descriptor::variable> extract_variable(scalar::expression& expr) {
        if (expr.kind() == scalar::variable_reference::tag) {
            return unsafe_downcast<scalar::variable_reference>(expr).variable();
        }
        return {};
    }
};

} // namespace

aggregate_analyzer::aggregate_analyzer(translator_type& translator)
    : translator_(translator)
{}

bool aggregate_analyzer::process(
        relation::project& projection,
        relation::intermediate::aggregate& aggregation,
        relation_info const& source) {
    group_members_.clear();
    group_members_.reserve(source.count_variables());
    source.enumerate_variables([&](descriptor::variable const& variable, std::string_view) {
        group_members_.emplace(variable);
    });
    for (auto&& v : aggregation.group_keys()) {
        group_members_.erase(v);
    }

    engine e { translator_, group_members_, aggregation };
    auto&& cs = projection.columns();
    for (auto it = cs.begin(); it != cs.end();) {
        if (!e.process(*it)) {
            return false;
        }
        if (!it->optional_value()) {
            it = cs.erase(it);
            continue;
        }
        ++it;
    }
    return true;
}

bool aggregate_analyzer::validate_group_member(relation_info const& output) {
    bool found {};
    output.enumerate_variables([&](descriptor::variable const& variable, std::string_view) {
        if (group_members_.find(variable) != group_members_.end()) {
            translator_.diagnostics().emplace_back(
                    code_type::invalid_aggregation_column,
                    "target column must be aggregated",
                    variable.region());
            found = true;
        }
    });
    return !found;
}


} // namespace mizugaki::translator::details
