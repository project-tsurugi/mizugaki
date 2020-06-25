#include <mizugaki/analyzer/details/analyze_scalar_expression.h>

#include <takatori/util/string_builder.h>

#include <mizugaki/ast/scalar/dispatch.h>

#include <mizugaki/analyzer/details/analyze_literal.h>
#include <mizugaki/analyzer/details/analyze_name.h>

namespace mizugaki::analyzer::details {

namespace tscalar = ::takatori::scalar;

using ::takatori::util::string_builder;

using result_type = analyze_scalar_expression_result;

namespace {

class engine {
public:
    explicit engine(
            analyzer_context& context,
            query_scope const& scope) noexcept :
        context_ { context },
        scope_ { scope }
    {}

    [[nodiscard]] result_type process(
            ast::scalar::expression const& expression,
            value_context const& value_context) {
        auto r = dispatch(expression, value_context);
        if (r) {
            return result_type {
                    std::move(r),
                    saw_aggregate_,
            };
        }
        return {};
    }

    [[nodiscard]] std::unique_ptr<tscalar::expression> operator()(
            ast::scalar::expression const& expr,
            value_context const&) {
        context_.report(
                sql_analyzer_code::unsupported_feature,
                string_builder {}
                        << "unsupported scalar expression: " << expr
                        << string_builder::to_string,
                expr.region());
        return {};
    }

    [[nodiscard]] std::unique_ptr<tscalar::expression> operator()(
            ast::scalar::literal_expression const& expr,
            value_context const& val) {
        return analyze_literal(context_, *expr.value(), val.find(0));
    }

    [[nodiscard]] std::unique_ptr<tscalar::expression> operator()(
            ast::scalar::variable_reference const& expr,
            value_context const&) {
        return analyze_variable_name(context_, *expr.name(), scope_);
    }

private:
    analyzer_context& context_;
    query_scope const& scope_;

    bool saw_aggregate_ { false };

    [[nodiscard]] std::unique_ptr<tscalar::expression> dispatch(
            ast::scalar::expression const& expression,
            value_context const& value_context) {
        return ast::scalar::dispatch(*this, expression, value_context);
    }
};

} // namespace

analyze_scalar_expression_result::analyze_scalar_expression_result(
        std::unique_ptr<tscalar::expression> expression,
        bool saw_aggregate) noexcept :
    expression_ { std::move(expression) },
    saw_aggregate_ { saw_aggregate }
{}

bool analyze_scalar_expression_result::has_value() const noexcept {
    return expression_ != nullptr;
}

analyze_scalar_expression_result::operator bool() const noexcept {
    return has_value();
}

tscalar::expression& analyze_scalar_expression_result::value() {
    return *expression_;
}

tscalar::expression const& analyze_scalar_expression_result::value() const {
    return *expression_;
}

tscalar::expression& analyze_scalar_expression_result::operator*() {
    return value();
}

tscalar::expression const& analyze_scalar_expression_result::operator*() const {
    return value();
}

std::unique_ptr<tscalar::expression> analyze_scalar_expression_result::release() {
    return std::move(expression_);
}

bool analyze_scalar_expression_result::saw_aggregate() const noexcept {
    return saw_aggregate_;
}

result_type analyze_scalar_expression(
        analyzer_context& context,
        ast::scalar::expression const& expression,
        query_scope const& scope,
        value_context const& value_context) {
    engine e { context, scope };
    return e.process(expression, value_context);
}

} // namespace mizugaki::analyzer::details
