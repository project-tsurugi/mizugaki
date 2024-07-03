#include "set_function_processor.h"

#include <takatori/descriptor/variable.h>

#include <takatori/scalar/dispatch.h>

#include <takatori/util/downcast.h>
#include <takatori/util/optional_ptr.h>
#include <takatori/util/string_builder.h>

#include <yugawara/binding/factory.h>

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

class engine {
public:
    explicit engine(
            analyzer_context& context,
            set_function_processor& processor) :
        context_ { context },
        processor_ { processor }
    {}

    [[nodiscard]] bool process(ownership_reference<tscalar::expression> ownership) {
        if (process(ownership.get(), 0)) {
            processor_.consume(std::move(ownership));
        }
        return !saw_error_;
    }

    [[nodiscard]] bool process(tscalar::expression& expr, std::size_t depth) {
        if (saw_error_) {
            return false;
        }
        return tscalar::dispatch(*this, expr, depth);
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

    bool operator()(tscalar::immediate const&, std::size_t) noexcept {
        return false;
    }

    bool operator()(tscalar::variable_reference const& expr, std::size_t depth) {
        if (depth == 0) {
            // NOTE: in aggregation depth = 0, you can use grouping keys or aggregated values
            if (!processor_.is_grouping(expr.variable()) && !processor_.is_aggregated(expr.variable())) {
                context_.report(
                        sql_analyzer_code::invalid_aggregation_column,
                        "column must be aggregated",
                        expr.region());
                saw_error_ = true;
            }
        } else {
            // NOTE: in aggregation depth > 0, using aggregated values may occur multi-level aggregations
            if (processor_.is_aggregated(expr.variable())) {
                context_.report(
                        sql_analyzer_code::invalid_aggregation_column,
                        "the aggregated result is not visible here",
                        expr.region());
                saw_error_ = true;
            }
        }
        return false;
    }

    bool operator()(tscalar::cast& expr, std::size_t depth) {
        if (process(expr.operand(), depth)) {
            processor_.consume(expr.ownership_operand());
        }
        return false;
    }

    bool operator()(tscalar::unary& expr, std::size_t depth) {
        if (process(expr.operand(), depth)) {
            processor_.consume(expr.ownership_operand());
        }
        return false;
    }

    bool operator()(tscalar::binary& expr, std::size_t depth) {
        if (process(expr.left(), depth)) {
            processor_.consume(expr.ownership_left());
        }
        if (process(expr.right(), depth)) {
            processor_.consume(expr.ownership_right());
        }
        return false;
    }

    bool operator()(tscalar::compare& expr, std::size_t depth) {
        if (process(expr.left(), depth)) {
            processor_.consume(expr.ownership_left());
        }
        if (process(expr.right(), depth)) {
            processor_.consume(expr.ownership_right());
        }
        return false;
    }

    bool operator()(tscalar::match& expr, std::size_t depth) {
        if (process(expr.input(), depth)) {
            processor_.consume(expr.ownership_input());
        }
        if (process(expr.pattern(), depth)) {
            processor_.consume(expr.ownership_pattern());
        }
        if (process(expr.escape(), depth)) {
            processor_.consume(expr.ownership_escape());
        }
        return false;
    }

    bool operator()(tscalar::conditional& expr, std::size_t depth) {
        for (auto&& element : expr.alternatives()) {
            if (process(element.condition(), depth)) {
                processor_.consume(element.ownership_condition());
            }
            if (process(element.body(), depth)) {
                processor_.consume(element.ownership_body());
            }
        }
        if (expr.default_expression() && process(*expr.default_expression(), depth)) {
            processor_.consume(expr.ownership_default_expression());
        }
        return false;
    }

    bool operator()(tscalar::coalesce& expr, std::size_t depth) {
        auto&& elements = expr.alternatives();
        for (auto it = elements.begin(); it != elements.end(); ++it) {
            if (process(*it, depth)) {
                processor_.consume(elements.ownership(it));
            }
        }
        return false;
    }

    bool operator()(tscalar::let& expr, std::size_t depth) {
        for (auto&& element : expr.variables()) {
            if (process(element.value(), depth)) {
                processor_.consume(element.ownership_value());
            }
        }
        if (process(expr.body(), depth)) {
            processor_.consume(expr.ownership_body());
        }
        return false;
    }

    bool operator()(tscalar::function_call& expr, std::size_t depth) {
        auto&& args = expr.arguments();
        for (auto it = args.begin(); it != args.end(); ++it) {
            if (process(*it, depth)) {
                processor_.consume(args.ownership(it));
            }
        }
        return false;
    }

    bool operator()(tscalar::extension& expr, std::size_t depth) {
        using ::yugawara::extension::scalar::extension_id;
        if (expr.extension_id() == extension_id::aggregate_function_call_id) {
            return operator()(unsafe_downcast<::yugawara::extension::scalar::aggregate_function_call>(expr), depth);
        }
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
            if (process(*it, depth + 1)) {
                // NOTE: may not come here cause of nested aggregation is not supported
                processor_.consume(args.ownership(it));
            }
        }
        return true;
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

void set_function_processor::add_aggregated(::takatori::descriptor::variable column) {
    aggregated_columns_.insert(std::move(column));
}

bool set_function_processor::is_grouping(tdescriptor::variable const& column) const {
    return grouping_columns_.find(column) != grouping_columns_.end();
}

bool set_function_processor::is_aggregated(tdescriptor::variable const& column) const {
    return aggregated_columns_.find(column) != aggregated_columns_.end();
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
    if (arguments_) {
        if (!context_.resolve(*arguments_)) {
            return {};
        }
        arguments_->input().connect_to(*current);
        current = arguments_->output();
        arguments_.reset();
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
        expression.set(std::make_unique<tscalar::variable_reference>(*existing));
    } else {
        // FIXME: add debug string
        auto replacement = ::yugawara::binding::factory {}.stream_variable();
        aggregations_store().columns().emplace_back(
                std::move(invocation.function()),
                std::move(arguments),
                replacement);
        expression.set(std::make_unique<tscalar::variable_reference>(replacement));
        aggregated_columns_.insert(std::move(replacement));
    }
}

::takatori::relation::project& set_function_processor::arguments_store() {
    if (!arguments_) {
        using ::takatori::relation::project;
        arguments_ = graph_.emplace<project>(std::vector<project::column> {});
    }
    return *arguments_;
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
    // FIXME: remove duplication
    // FIXME: add debug string
    auto replacement = ::yugawara::binding::factory {}.stream_variable();
    auto&& origin = expression.exchange(
            std::make_unique<tscalar::variable_reference>(replacement));
    replacement.region() = origin->region();
    arguments_store().columns().emplace_back(std::move(origin), replacement);
    return replacement;
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
        for (std::size_t position = 0, size = arguments.size(); position < size; ++position) {
            if (entry.arguments()[position] != arguments[position]) {
                continue;
            }
        }
        return entry.destination();
    }
    return {};
}

} // namespace mizugaki::analyzer::details
