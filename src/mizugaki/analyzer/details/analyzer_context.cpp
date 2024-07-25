#include <mizugaki/analyzer/details/analyzer_context.h>

#include <takatori/util/exception.h>

namespace mizugaki::analyzer::details {

using ::takatori::util::finalizer;
using ::takatori::util::optional_ptr;
using ::takatori::util::sequence_view;
using ::takatori::util::throw_exception;

finalizer analyzer_context::initialize(
        options_type const& options,
        optional_ptr<::takatori::document::document const> source,
        sequence_view<ast::node_region const> comments,
        placeholder_map const& placeholders,
        optional_ptr<::yugawara::variable::provider const> host_parameters) {
    bool b = false;
    if (!initialized_.compare_exchange_strong(
            b, true,
            std::memory_order_acquire, std::memory_order_relaxed)) {
        throw_exception(std::runtime_error("analyzer context is in use"));
    }

    finalizer f {
        [this] { finalize(); }
    };

    options_ = options;
    source_ = source;
    comments_ = comments;
    placeholders_ = placeholders;
    host_parameters_ = host_parameters;

    diagnostics_.clear();
    types_.clear();
    values_.clear();
    expression_analyzer_.clear_diagnostics();
    expression_analyzer_.variables().clear();
    expression_analyzer_.expressions().clear();
    expression_analyzer_.allow_unresolved(true);

    return f;
}

void analyzer_context::finalize() {
    options_ = {};
    source_ = {};
    comments_ = {};
    placeholders_ = {};
    host_parameters_ = {};

    diagnostics_.clear();
    types_.clear();
    values_.clear();
    expression_analyzer_.clear_diagnostics();
    expression_analyzer_.variables().clear();
    expression_analyzer_.expressions().clear();

    initialized_.store(false, std::memory_order_release);
}

std::shared_ptr<::takatori::type::data const>
analyzer_context::resolve(::takatori::scalar::expression const& expression, bool validate) {
    auto result = expression_analyzer_.resolve(expression, validate, types_);
    if (expression_analyzer_.has_diagnostics()) {
        for (auto&& d : expression_analyzer_.diagnostics()) {
            auto code = convert_code(d.code());
            report(code, d.message(), d.location());
        }
        expression_analyzer_.clear_diagnostics();
        return {};
    }
    if (!result) {
        report(sql_analyzer_code::unknown,
                "unknown error occurred while analyzing type of scalar expression",
                expression.region());
        return {};
    }
    return result;
}

bool analyzer_context::resolve(::takatori::relation::expression const& expression, bool validate) {
    auto result = expression_analyzer_.resolve(expression, validate, false, types_);
    if (expression_analyzer_.has_diagnostics()) {
        for (auto&& d : expression_analyzer_.diagnostics()) {
            auto code = convert_code(d.code());
            report(code, d.message(), d.location());
        }
        expression_analyzer_.clear_diagnostics();
        return false;
    }
    if (!result) {
        report(sql_analyzer_code::unknown,
                "unknown error occurred while analyzing type of scalar expression",
                expression.region());
        return false;
    }
    return true;
}

void analyzer_context::clear_expression_resolution() {
    expression_analyzer_.expressions().clear();
}

void analyzer_context::clear_expression_resolution(::takatori::scalar::expression const& expression) {
    expression_analyzer_.expressions().unbind(expression);
}

void analyzer_context::resolve_as(
        ::takatori::descriptor::variable const& variable,
        ::yugawara::analyzer::variable_resolution resolution) {
    expression_analyzer_.variables().bind(variable, std::move(resolution), true);
}

::yugawara::compiled_info analyzer_context::test_info() {
    return {
            expression_analyzer_.shared_expressions().ownership(),
            expression_analyzer_.shared_variables().ownership(),
    };
}

sql_analyzer_code analyzer_context::convert_code(
        ::yugawara::analyzer::expression_analyzer_code code) noexcept {
    using kind = decltype(code);
    switch (code) {
        case kind::unknown: return sql_analyzer_code::unknown;
        case kind::unsupported_type: return sql_analyzer_code::unsupported_feature;
        case kind::ambiguous_type: return sql_analyzer_code::ambiguous_type;
        case kind::inconsistent_type: return sql_analyzer_code::inconsistent_type;
        case kind::unresolved_variable: return sql_analyzer_code::unresolved_variable;
        case kind::inconsistent_elements: return sql_analyzer_code::inconsistent_elements;
    }
    return sql_analyzer_code::unknown;
}

::takatori::document::region analyzer_context::convert(ast::node_region region) const {
    if (source_) {
        return diagnostic_type::location_type {
                *source_,
                region.begin,
                region.end,
        };
    }
    return {};
}

void analyzer_context::report(
        diagnostic_type::code_type code,
        diagnostic_type::message_type message,
        ::takatori::document::region region) {
    diagnostics_.emplace_back(code, std::move(message), region);
}

void analyzer_context::report(
        diagnostic_type::code_type code,
        diagnostic_type::message_type message,
        ast::node_region region) {
    diagnostics_.emplace_back(code, std::move(message), convert(region));
}

} // namespace mizugaki::analyzer::details
