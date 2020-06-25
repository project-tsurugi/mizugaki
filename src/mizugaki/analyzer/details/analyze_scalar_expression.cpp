#include <mizugaki/analyzer/details/analyze_scalar_expression.h>

#include <takatori/scalar/cast.h>
#include <takatori/scalar/unary.h>
#include <takatori/scalar/binary.h>
#include <takatori/scalar/compare.h>
#include <takatori/scalar/variable_reference.h>

#include <takatori/util/string_builder.h>
#include <takatori/util/downcast.h>

#include <yugawara/binding/factory.h>

#include <mizugaki/ast/scalar/dispatch.h>
#include <mizugaki/ast/literal/boolean.h>

#include <mizugaki/analyzer/details/analyze_literal.h>
#include <mizugaki/analyzer/details/analyze_name.h>
#include <mizugaki/analyzer/details/analyze_type.h>

namespace mizugaki::analyzer::details {

namespace tscalar = ::takatori::scalar;

using ::takatori::util::unsafe_downcast;
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

    [[nodiscard]] std::unique_ptr<tscalar::expression> operator()(
            ast::scalar::host_parameter_reference const& expr,
            value_context const&) {
        auto&& identifier = expr.name()->identifier();
        if (auto placeholders = context_.placeholders()) {
            if (auto value = placeholders->find(identifier)) {
                auto result = value->resolve();
                result->region() = context_.convert(expr.region());
                return result;
            }
        }
        if (auto host_variables = context_.host_variables()) {
            if (auto variable = host_variables->find(identifier)) {
                auto descriptor = context_.bless(factory_(std::move(variable)), expr.region());
                return context_.create<tscalar::variable_reference>(
                        expr.region(),
                        std::move(descriptor));
            }
        }
        context_.report(
                sql_analyzer_code::variable_not_found,
                string_builder {}
                        << "placeholder is not found: "
                        << expr.name()->identifier()
                        << string_builder::to_string,
                expr.region());
        return {};
    }

    [[nodiscard]] std::unique_ptr<tscalar::expression> operator()(
            ast::scalar::cast_expression const& expr,
            value_context const&) {
        auto type = analyze_type(context_, *expr.type());
        if (!type) {
            return {};
        }
        auto operand = process(*expr.operand(), {});
        if (!operand) {
            return {};
        }
        auto result = context_.create<tscalar::cast>(
                expr.operand()->region(),
                std::move(type),
                tscalar::cast::loss_policy_type::ignore,
                operand.release());
        return result;
    }

    [[nodiscard]] std::unique_ptr<tscalar::expression> operator()(
            ast::scalar::unary_expression const& expr,
            value_context const&) {
        auto operator_ = convert(expr.operator_kind());
        if (!operator_) {
            return {};
        }
        auto operand = process(*expr.operand(), {});
        if (!operand) {
            return {};
        }
        auto result = context_.create<tscalar::unary>(
                expr.region(),
                *operator_,
                operand.release());
        return result;
    }

    [[nodiscard]] std::optional<tscalar::unary_operator> convert(
            ast::common::regioned<ast::scalar::unary_operator> const& source) {
        using from = ast::scalar::unary_operator;
        using to = tscalar::unary_operator;
        switch (*source) {
            case from::plus:
                return to::plus;
            case from::minus:
                return to::sign_inversion;
            case from::not_:
                return to::conditional_not;
            default:
                break;
        }
        context_.report(
                sql_analyzer_code::unsupported_feature,
                string_builder {}
                        << "unsupported unary operator: " << source
                        << string_builder::to_string,
                source.region());
        return {};
    }

    [[nodiscard]] std::unique_ptr<tscalar::expression> operator()(
            ast::scalar::binary_expression const& expr,
            value_context const&) {
        if (expr.operator_kind() == ast::scalar::binary_operator::is ||
                expr.operator_kind() == ast::scalar::binary_operator::is_not) {
            return process_is(expr, {});
        }

        auto operator_ = convert(expr.operator_kind());
        if (!operator_) {
            return {};
        }
        auto left = process(*expr.left(), {});
        if (!left) {
            return {};
        }
        auto right = process(*expr.right(), {});
        if (!right) {
            return {};
        }
        auto result = context_.create<tscalar::binary>(
                expr.region(),
                *operator_,
                left.release(),
                right.release());
        return result;
    }

    [[nodiscard]] std::optional<tscalar::binary_operator> convert(
            ast::common::regioned<ast::scalar::binary_operator> const& source) {
        using from = ast::scalar::binary_operator;
        using to = tscalar::binary_operator;
        switch (*source) {
            case from::plus:
                return to::add;
            case from::minus:
                return to::subtract;
            case from::asterisk:
                return to::multiply;
            case from::solidus:
                return to::divide;
            case from::concatenation:
                return to::concat;
            case from::and_:
                return to::conditional_and;
            case from::or_:
                return to::conditional_or;
            default:
                break;
        }
        context_.report(
                sql_analyzer_code::unsupported_feature,
                string_builder {}
                        << "unsupported unary operator: " << source
                        << string_builder::to_string,
                source.region());
        return {};
    }

    [[nodiscard]] std::unique_ptr<tscalar::expression> process_is(
            ast::scalar::binary_expression const& expr,
            value_context const&) {
        auto&& right = *expr.right();
        if (right.node_kind() != ast::scalar::kind::literal_expression) {
            context_.report(
                    sql_analyzer_code::unsupported_feature,
                    string_builder {}
                            << "unsupported right operand for IS/IS NOT: "
                            << right.node_kind()
                            << string_builder::to_string,
                    right.region());
            return {};
        }
        auto&& literal = unsafe_downcast<ast::scalar::literal_expression>(right);
        auto operator_ = convert_is_right(*literal.value());
        if (!operator_) {
            return {};
        }
        auto left = process(*expr.left(), {});
        if (!left) {
            return {};
        }
        auto result = context_.create<tscalar::unary>(
                expr.region(),
                *operator_,
                left.release());
        if (expr.operator_kind() == ast::scalar::binary_operator::is_not) {
            // negate the logic
            result = context_.create<tscalar::unary>(
                    expr.region(),
                    tscalar::unary_operator::conditional_not,
                    std::move(result));
        }
        return result;
    }

    [[nodiscard]] std::optional<tscalar::unary_operator> convert_is_right(
            ast::literal::literal const& source) {
        using from = ast::literal::kind;
        using to = tscalar::unary_operator;
        switch (source.node_kind()) {
            case from::null:
                return to::is_null;
            case from::boolean:
                {
                    auto&& value = unsafe_downcast<ast::literal::boolean>(source);
                    switch (value.value()) {
                        case ast::literal::boolean_kind::true_:
                            return to::is_true;
                        case ast::literal::boolean_kind::false_:
                            return to::is_false;
                        case ast::literal::boolean_kind::unknown:
                            return to::is_unknown;
                        default:
                            break;
                    }
                }
                break;
            default:
                break;
        }
        context_.report(
                sql_analyzer_code::unsupported_feature,
                string_builder {}
                        << "unsupported right operand for IS/IS NOT: "
                        << source.node_kind()
                        << string_builder::to_string,
                source.region());
        return {};
    }

    [[nodiscard]] std::unique_ptr<tscalar::expression> operator()(
            ast::scalar::comparison_predicate const& expr,
            value_context const&) {
        auto operator_ = convert(expr.operator_kind());
        if (!operator_) {
            return {};
        }
        auto left = process(*expr.left(), {});
        if (!left) {
            return {};
        }
        auto right = process(*expr.right(), {});
        if (!right) {
            return {};
        }
        auto result = context_.create<tscalar::compare>(
                expr.region(),
                *operator_,
                left.release(),
                right.release());
        return result;
    }

    [[nodiscard]] std::optional<tscalar::comparison_operator> convert(
            ast::common::regioned<ast::scalar::comparison_operator> const& source) {
        using from = ast::scalar::comparison_operator;
        using to = tscalar::comparison_operator;
        switch (*source) {
            case from::equals:
                return to::equal;
            case from::not_equals:
                return to::not_equal;
            case from::less_than:
                return to::less;
            case from::less_than_or_equals:
                return to::less_equal;
            case from::greater_than:
                return to::greater;
            case from::greater_than_or_equals:
                return to::greater_equal;
            default:
                break;
        }
        context_.report(
                sql_analyzer_code::unsupported_feature,
                string_builder {}
                        << "unsupported comparison operator: " << source
                        << string_builder::to_string,
                source.region());
        return {};
    }

private:
    analyzer_context& context_;
    query_scope const& scope_;

    ::yugawara::binding::factory factory_;

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
