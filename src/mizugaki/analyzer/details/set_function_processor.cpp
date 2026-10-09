#include "set_function_processor.h"

#include <takatori/descriptor/variable.h>

#include <takatori/scalar/dispatch.h>

#include <takatori/util/downcast.h>
#include <takatori/util/optional_ptr.h>
#include <takatori/util/string_builder.h>

#include <yugawara/binding/extract.h>
#include <yugawara/binding/factory.h>
#include <yugawara/binding/variable_info.h>

#include <yugawara/extension/scalar/aggregate_function_call.h>
#include <yugawara/extension/scalar/subquery.h>
#include <yugawara/extension/scalar/exists.h>
#include <yugawara/extension/scalar/quantified_compare.h>

namespace mizugaki::analyzer::details {

namespace tdescriptor = ::takatori::descriptor;
namespace tscalar = ::takatori::scalar;

using output_port_type = ::takatori::relation::expression::output_port_type;

using ::takatori::util::ownership_reference;

using ::takatori::util::downcast;
using ::takatori::util::optional_ptr;
using ::takatori::util::ownership_reference;
using ::takatori::util::string_builder;
using ::takatori::util::unsafe_downcast;

namespace {

enum class inspect_result_kind {
    none,
    group_key,
    aggregate,
};

class inspect_result {
public:
    using kind_type = inspect_result_kind;

    constexpr inspect_result() = default;

    static inspect_result aggregate() noexcept {
        return inspect_result { kind_type::aggregate, std::nullopt };
    }

    static inspect_result group_key(tdescriptor::variable column) noexcept {
        return inspect_result { kind_type::group_key, std::move(column) };
    }

    [[nodiscard]] kind_type kind() const noexcept {
        return kind_;
    }

    [[nodiscard]] std::optional<tdescriptor::variable>& group_key() noexcept {
        return group_key_;
    }

    [[nodiscard]] std::optional<tdescriptor::variable> const& group_key() const noexcept {
        return group_key_;
    }

    explicit operator bool() const noexcept {
        return kind_ != kind_type::none;
    }

private:
    explicit inspect_result(kind_type kind, std::optional<tdescriptor::variable> group_key) noexcept :
        kind_ { kind },
        group_key_ { std::move(group_key) }
    {}

    kind_type kind_ { kind_type::none };
    std::optional<tdescriptor::variable> group_key_ {};
};

class engine {
public:
    explicit engine(
            analyzer_context& context,
            set_function_processor& processor) :
        context_ { context },
        processor_ { processor }
    {}

    [[nodiscard]] bool process(ownership_reference<tscalar::expression> ownership) {
        if (auto result = inspect(ownership.get(), 0)) {
            replace(std::move(ownership), std::move(result));
        }
        return !saw_error_;
    }

    [[nodiscard]] inspect_result inspect(tscalar::expression& expr, std::size_t depth) {
        if (saw_error_) {
            return {};
        }
        // NOTE: check extracted group keys before inspecting the operands, so that the largest matching
        //       sub-expression is replaced with a group key rather than its operands
        //       (e.g., `a + 1 + b` matches `GROUP BY a + 1, a + 1 + b` as a whole, not `a + 1` and `b`).
        if (depth == 0) {
            if (auto existing = processor_.find_extracted_group_key(expr)) {
                return inspect_result::group_key(std::move(*existing));
            }
        }
        if (tscalar::dispatch(*this, expr, depth)) {
            return inspect_result::aggregate();
        }
        return {};
    }

    void replace(ownership_reference<tscalar::expression> expression, inspect_result result) {
        switch (result.kind()) {
            case inspect_result::kind_type::none:
                return;
            case inspect_result::kind_type::group_key: {
                auto&& replacement = context_.create<tscalar::variable_reference>(
                        expression->region(),
                        std::move(*result.group_key())); // NOLINT
                auto&& origin = expression.exchange(std::move(replacement));
                context_.clear_expression_resolution(std::move(origin));
                return;
            }
            case inspect_result::kind_type::aggregate:
                processor_.consume(std::move(expression));
                return;
        }
    }

    bool operator()(tscalar::expression const& expr, std::size_t) {
        context_.report(
                sql_analyzer_code::unsupported_feature,
                string_builder {}
                        << "unknown expression while analyzing aggregate operations: "
                        << expr.kind()
                        << string_builder::to_string,
                expr.region());
        saw_error_ = true;
        return false;
    }

    bool operator()(tscalar::immediate const& expr, std::size_t depth) noexcept {
        (void) expr;
        (void) depth;
        return false;
    }

    bool operator()(tscalar::variable_reference const& expr, std::size_t depth) {
        // check only if it is stream variable
        auto&& variable_info = ::yugawara::binding::unwrap(expr.variable());
        if (variable_info.kind() != ::yugawara::binding::variable_info_kind::stream_variable) {
            return false;
        }

        check_variable(expr.region(), expr.variable(), depth);
        return false;
    }

    void check_variable(
            ::takatori::document::region const& region,
            tdescriptor::variable const& variable,
            std::size_t depth) {
        if (depth == 0) {
            // NOTE: in aggregation depth = 0, you can use grouping keys or aggregated values
            if (!processor_.is_grouping(variable) && !processor_.is_aggregated(variable)) {
                context_.report(
                        sql_analyzer_code::invalid_aggregation_column,
                        "column must be aggregated",
                        region);
                saw_error_ = true;
            }
        } else {
            // NOTE: in aggregation depth > 0, using aggregated values may occur multi-level aggregations
            if (processor_.is_aggregated(variable)) {
                context_.report(
                        sql_analyzer_code::invalid_aggregation_column,
                        "the aggregated result is not visible here",
                        region);
                saw_error_ = true;
            }
        }
    }

    bool operator()(tscalar::cast& expr, std::size_t depth) {
        if (auto result = inspect(expr.operand(), depth)) {
            replace(expr.ownership_operand(), std::move(result));
        }
        return false;
    }

    bool operator()(tscalar::unary& expr, std::size_t depth) {
        if (auto result = inspect(expr.operand(), depth)) {
            replace(expr.ownership_operand(), std::move(result));
        }
        return false;
    }

    bool operator()(tscalar::binary& expr, std::size_t depth) {
        if (auto result = inspect(expr.left(), depth)) {
            replace(expr.ownership_left(), std::move(result));
        }
        if (auto result = inspect(expr.right(), depth)) {
            replace(expr.ownership_right(), std::move(result));
        }
        return false;
    }

    bool operator()(tscalar::compare& expr, std::size_t depth) {
        if (auto result = inspect(expr.left(), depth)) {
            replace(expr.ownership_left(), std::move(result));
        }
        if (auto result = inspect(expr.right(), depth)) {
            replace(expr.ownership_right(), std::move(result));
        }
        return false;
    }

    bool operator()(tscalar::match& expr, std::size_t depth) {
        if (auto result = inspect(expr.input(), depth)) {
            replace(expr.ownership_input(), std::move(result));
        }
        if (auto result = inspect(expr.pattern(), depth)) {
            replace(expr.ownership_pattern(), std::move(result));
        }
        if (auto result = inspect(expr.escape(), depth)) {
            replace(expr.ownership_escape(), std::move(result));
        }
        return false;
    }

    bool operator()(tscalar::conditional& expr, std::size_t depth) {
        for (auto&& element : expr.alternatives()) {
            if (auto result = inspect(element.condition(), depth)) {
                replace(element.ownership_condition(), std::move(result));
            }
            if (auto result = inspect(element.body(), depth)) {
                replace(element.ownership_body(), std::move(result));
            }
        }
        if (expr.default_expression()) {
            if (auto result = inspect(*expr.default_expression(), depth)) {
                replace(expr.ownership_default_expression(), std::move(result));
            }
        }
        return false;
    }

    bool operator()(tscalar::coalesce& expr, std::size_t depth) {
        auto&& elements = expr.alternatives();
        for (auto it = elements.begin(); it != elements.end(); ++it) {
            if (auto result = inspect(*it, depth)) {
                replace(elements.ownership(it), std::move(result));
            }
        }
        return false;
    }

    bool operator()(tscalar::let& expr, std::size_t depth) {
        for (auto&& element : expr.variables()) {
            if (auto result = inspect(element.value(), depth)) {
                replace(element.ownership_value(), std::move(result));
            }
        }
        if (auto result = inspect(expr.body(), depth)) {
            replace(expr.ownership_body(), std::move(result));
        }
        return false;
    }

    bool operator()(tscalar::function_call& expr, std::size_t depth) {
        auto&& args = expr.arguments();
        for (auto it = args.begin(); it != args.end(); ++it) {
            if (auto result = inspect(*it, depth)) {
                replace(args.ownership(it), std::move(result));
            }
        }
        return false;
    }

    bool operator()(tscalar::extension& expr, std::size_t depth) {
        using namespace ::yugawara::extension::scalar;
        switch (expr.extension_id()) {
            case aggregate_function_call::extension_tag:
                return operator()(unsafe_downcast<aggregate_function_call>(expr), depth);
            case subquery::extension_tag:
                return operator()(unsafe_downcast<subquery>(expr), depth);
            case exists::extension_tag:
                return operator()(unsafe_downcast<exists>(expr), depth);
            case quantified_compare::extension_tag:
                return operator()(unsafe_downcast<quantified_compare>(expr), depth);
            default:
                context_.report(
                        sql_analyzer_code::unsupported_feature,
                        string_builder {}
                                << "unknown scalar expression extension: "
                                << expr.extension_id()
                                << string_builder::to_string,
                        expr.region());
                saw_error_ = true;
            return false;
        }
    }

    bool operator()(::yugawara::extension::scalar::aggregate_function_call& expr, std::size_t depth) {
        if (depth > 0) {
            context_.report(
                    sql_analyzer_code::unsupported_feature,
                    "nested aggregation is not supported",
                    expr.region());
            saw_error_ = true;
            return false;
        }
        auto&& args = expr.arguments();
        for (auto it = args.begin(); it != args.end(); ++it) {
            // we try to check individual variables are visible
            if (auto result = inspect(*it, depth + 1)) {
                // NOTE: may not come here cause of nested aggregation is not supported
                replace(args.ownership(it), std::move(result));
            }
        }
        return true;
    }

    bool operator()(::yugawara::extension::scalar::subquery const& expr, std::size_t depth) noexcept {
        for (auto&& parameter : expr.parameters()) {
            check_variable(expr.region(), parameter.source(), depth);
        }
        return false;
    }

    bool operator()(::yugawara::extension::scalar::exists const& expr, std::size_t depth) noexcept {
        for (auto&& parameter : expr.parameters()) {
            check_variable(expr.region(), parameter.source(), depth);
        }
        return false;
    }

    bool operator()(::yugawara::extension::scalar::quantified_compare& expr, std::size_t depth) {
        for (auto&& parameter : expr.parameters()) {
            check_variable(expr.region(), parameter.source(), depth);
        }
        if (auto result = inspect(expr.left(), depth)) {
            replace(expr.ownership_left(), std::move(result));
        }
        return false;
    }

private:
    analyzer_context& context_;
    set_function_processor& processor_;
    bool saw_error_ { false };
};

} // namespace

set_function_processor::set_function_processor(
        analyzer_context &context,
        takatori::relation::graph_type &graph) :
    context_ { context },
    graph_ { graph }
{}

void set_function_processor::add_group_key(tdescriptor::variable column) {
    auto existing = grouping_columns_.find(column);
    if (existing != grouping_columns_.end()) {
        // ignore exiting group key
        return;
    }
    activate();
    grouping_columns_.insert(column);
    aggregations_store().group_keys().emplace_back(std::move(column));
}

void set_function_processor::add_group_key(std::unique_ptr<takatori::scalar::expression> expression) {
    if (expression->kind() == tscalar::variable_reference::tag) {
        auto&& var_ref = unsafe_downcast<tscalar::variable_reference>(*expression);
        add_group_key(var_ref.variable());
        context_.clear_expression_resolution(std::move(expression));
        return;
    }
    if (auto existing = find_extracted_expression(*expression)) {
        add_group_key(std::move(*existing));
        context_.clear_expression_resolution(std::move(expression));
        return;
    }
    auto replacement = ::yugawara::binding::factory {}.stream_variable();
    replacement.region() = expression->region();
    extracted_expressions_store().columns().emplace_back(std::move(expression), replacement);
    add_group_key(std::move(replacement));
}

std::optional<tdescriptor::variable> set_function_processor::find_extracted_group_key(
        tscalar::expression const& expression) const {
    // NOTE: short-circuit - following expression kinds are never extracted:
    //   - immediate values - they do not depend on any input columns, so they can be used as is
    //   - variable references - add_group_key() registers them as group keys directly,
    //                           instead of placing them into extracted_expressions_store()
    static constexpr tscalar::expression_kind_set never_extracted_kinds {
            tscalar::immediate::tag,
            tscalar::variable_reference::tag,
    };
    if (never_extracted_kinds.contains(expression.kind())) {
        return std::nullopt;
    }
    if (auto existing = find_extracted_expression(expression); existing && is_grouping(*existing)) {
        return existing;
    }
    return std::nullopt;
}

void set_function_processor::add_aggregated(::takatori::descriptor::variable column) {
    aggregated_columns_.insert(std::move(column));
}

bool set_function_processor::is_grouping(tdescriptor::variable const& column) const {
    return grouping_columns_.contains(column);
}

bool set_function_processor::is_aggregated(tdescriptor::variable const& column) const {
    return aggregated_columns_.contains(column);
}

bool set_function_processor::active() const noexcept {
    return active_;
}

void set_function_processor::activate() noexcept {
    active_ = true;
}

bool set_function_processor::process(ownership_reference<::takatori::scalar::expression> expression) {
    engine e { context_, *this };
    return e.process(std::move(expression));
}

optional_ptr<output_port_type> set_function_processor::install(output_port_type& port) {
    auto next = port.opposite();
    if (next) {
        next->disconnect_from(port);
    }
    optional_ptr<output_port_type> current { port };
    if (extracted_expressions_) {
        if (!context_.resolve(*extracted_expressions_)) {
            return {};
        }
        extracted_expressions_->input().connect_to(*current);
        current = extracted_expressions_->output();
        extracted_expressions_.reset();
    }
    if (aggregations_) {
        if (!context_.resolve(*aggregations_)) {
            return {};
        }
        aggregations_->input().connect_to(*current);
        current = aggregations_->output();
        aggregations_.reset();
    }
    if (next) {
        current->connect_to(*next);
    }
    active_ = false;
    grouping_columns_.clear();
    aggregated_columns_.clear();
    return *current;
}

void set_function_processor::consume(ownership_reference<tscalar::expression> expression) {
    activate();
    auto&& invocation = downcast<::yugawara::extension::scalar::aggregate_function_call>(expression.get());

    std::vector<tdescriptor::variable> arguments {};
    arguments.reserve(invocation.arguments().size());
    for (auto iter = invocation.arguments().begin(); iter != invocation.arguments().end(); ++iter) {
        auto ownership = invocation.arguments().ownership(iter);
        auto replacement = replace_argument(std::move(ownership));
        arguments.emplace_back(std::move(replacement));
    }

    auto existing = find_aggregation(invocation.function(), arguments);
    if (existing) {
        auto&& origin = expression.exchange(std::make_unique<tscalar::variable_reference>(*existing));
        context_.clear_expression_resolution(std::move(origin));
    } else {
        // FIXME: add debug string
        auto replacement = ::yugawara::binding::factory {}.stream_variable();
        aggregations_store().columns().emplace_back(
                std::move(invocation.function()),
                std::move(arguments),
                replacement);
        auto&& origin = expression.exchange(std::make_unique<tscalar::variable_reference>(replacement));
        context_.clear_expression_resolution(std::move(origin));
        aggregated_columns_.insert(std::move(replacement));
    }
}

::takatori::relation::project& set_function_processor::extracted_expressions_store() {
    if (!extracted_expressions_) {
        using ::takatori::relation::project;
        extracted_expressions_ = graph_.emplace<project>(std::vector<project::column> {});
    }
    return *extracted_expressions_;
}

::takatori::relation::intermediate::aggregate &set_function_processor::aggregations_store() {
    if (!aggregations_) {
        using ::takatori::relation::intermediate::aggregate;
        aggregations_ = graph_.emplace<aggregate>(
                std::vector<tdescriptor::variable> {},
                std::vector<aggregate::column> {});
    }
    return *aggregations_;
}

tdescriptor::variable set_function_processor::replace_argument(ownership_reference<tscalar::expression> expression) {
    if (auto existing = find_extracted_expression(*expression)) {
        existing->region() = expression->region();
        auto replacement_expr = context_.create<tscalar::variable_reference>(
                expression->region(),
                *existing);
        auto origin = expression.exchange(std::move(replacement_expr));
        context_.clear_expression_resolution(std::move(origin));
        return std::move(*existing);
    }

    // FIXME: add debug string
    auto replacement_var = ::yugawara::binding::factory {}.stream_variable();
    replacement_var.region() = expression->region();
    auto replacement_expr = context_.create<tscalar::variable_reference>(
            expression->region(),
            replacement_var);
    auto&& origin = expression.exchange(std::move(replacement_expr));
    extracted_expressions_store().columns().emplace_back(std::move(origin), replacement_var);
    return replacement_var;
}

std::optional<tdescriptor::variable> set_function_processor::find_extracted_expression(tscalar::expression const& expression) const {
    if (!extracted_expressions_) {
        return {};
    }
    for (auto&& column : extracted_expressions_->columns()) {
        if (column.value() == expression) {
            return column.variable();
        }
    }
    return {};
}

std::optional<tdescriptor::variable>
set_function_processor::find_aggregation(
        tdescriptor::aggregate_function const& function,
        std::vector<tdescriptor::variable> const& arguments) {
    for (auto&& entry : aggregations_store().columns()) {
        if (entry.function() != function) {
            continue;
        }
        if (entry.arguments().size() != arguments.size()) {
            continue;
        }
        bool argument_match = true;
        for (std::size_t position = 0, size = arguments.size(); position < size; ++position) {
            if (entry.arguments()[position] != arguments[position]) {
                argument_match = false;
                break;
            }
        }
        if (!argument_match) {
            continue;
        }
        return entry.destination();
    }
    return {};
}

} // namespace mizugaki::analyzer::details
