#include <mizugaki/analyzer/details/analyze_scalar_expression.h>

#include <vector>

#include <takatori/type/boolean.h>

#include <takatori/scalar/cast.h>
#include <takatori/scalar/unary.h>
#include <takatori/scalar/binary.h>
#include <takatori/scalar/compare.h>
#include <takatori/scalar/variable_reference.h>
#include <takatori/scalar/let.h>
#include <takatori/scalar/function_call.h>

#include <takatori/util/string_builder.h>
#include <takatori/util/downcast.h>

#include <yugawara/binding/factory.h>
#include <yugawara/type/conversion.h>

#include <yugawara/extension/scalar/aggregate_function_call.h>

#include <mizugaki/ast/scalar/dispatch.h>
#include <mizugaki/ast/literal/boolean.h>

#include <mizugaki/analyzer/details/analyze_literal.h>
#include <mizugaki/analyzer/details/analyze_name.h>
#include <mizugaki/analyzer/details/analyze_type.h>

#include "name_print_support.h"

namespace mizugaki::analyzer::details {

namespace tscalar = ::takatori::scalar;
namespace ttype = ::takatori::type;

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
        if (!r) {
            return {};
        }
        if (!validate(*r)) {
            return {};
        }
        return result_type {
                std::move(r),
                saw_aggregate_,
        };
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
        // NOTE: keep the case of host parameter names
        std::string_view identifier = expr.name()->identifier();
        if (!context_.options()->host_parameter_declaration_starts_with_colon()) {
            // The host parameter looking for does not start with colon.
            // We try to trim the leading colon in the expression.
            if (!identifier.empty() && identifier[0] == ':') {
                identifier.remove_prefix(1);
            }
        }
        if (auto placeholders = context_.placeholders()) {
            if (auto value = placeholders->find(identifier)) {
                auto result = value->resolve();
                result->region() = context_.convert(expr.region());
                return result;
            }
        }
        if (auto host_parameters = context_.host_parameters()) {
            if (auto variable = host_parameters->find(identifier)) {
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
                        << print_support { *expr.name() }
                        << string_builder::to_string,
                expr.region());
        return {};
    }

    // FIXME: impl field reference
    // FIXME: impl case expression

    [[nodiscard]] std::unique_ptr<tscalar::expression> operator()(
            ast::scalar::cast_expression const& expr,
            value_context const&) {
        if (expr.operator_kind() != ast::scalar::cast_operator::cast) {
            context_.report(
                    sql_analyzer_code::unsupported_feature,
                    string_builder {}
                            << "unsupported cast type: "
                            << expr.operator_kind()
                            << string_builder::to_string,
                    expr.operator_kind().region());
        }
        auto operand = process(*expr.operand(), {});
        if (!operand) {
            return {};
        }
        auto type = analyze_type(context_, *expr.type());
        if (!type) {
            return {};
        }
        auto result = context_.create<tscalar::cast>(
                expr.operand()->region(),
                std::move(type),
                tscalar::cast::loss_policy_type::ignore,
                operand.release());
        return result;
    }

    // FIXME: impl unary AT LOCAL
    // FIXME: impl unary DEFEF

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

    // FIXME: impl binary []
    // FIXME: impl AT TIME ZONE

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
            case from::percent:
                return to::remainder;
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
                sql_analyzer_code::malformed_syntax,
                string_builder {}
                        << "unsupported right operand for IS/IS NOT: "
                        << source.node_kind()
                        << string_builder::to_string,
                source.region());
        return {};
    }

    // FIXME: impl OVERLAPS
    // FIXME: impl DISTINCT FROM
    // FIXME: impl <@
    // FIXME: impl @>
    // FIXME: impl &&

    // FIXME: impl extract_expression
    // FIXME: impl trim_expression
    // FIXME: impl value_constructor
    // FIXME: impl subquery (scalar/row)

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

    // FIXME: impl quantified_comparison_predicate,

    [[nodiscard]] std::unique_ptr<tscalar::expression> operator()(
            ast::scalar::between_predicate const& expr,
            value_context const&) {
        auto target = process(*expr.target(), {});
        if (!target) {
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

        std::unique_ptr<tscalar::expression> result {};
        if (expr.operator_kind() == ast::scalar::between_operator::symmetric) {
            auto v_target = factory_.stream_variable();
            auto v_left = factory_.stream_variable();
            auto v_right = factory_.stream_variable();

            std::vector<tscalar::let::variable> variables {};
            variables.reserve(3);
            variables.emplace_back(v_target, target.release());
            variables.emplace_back(v_left, left.release());
            variables.emplace_back(v_right, right.release());

            auto forward = context_.create<tscalar::binary>(
                    expr.region(),
                    tscalar::binary_operator::conditional_and,
                    context_.create<tscalar::compare>(
                            expr.region(),
                            tscalar::comparison_operator::less_equal,
                            context_.create<tscalar::variable_reference>(expr.left()->region(), v_left),
                            context_.create<tscalar::variable_reference>(expr.target()->region(), v_target)),
                    context_.create<tscalar::compare>(
                            expr.region(),
                            tscalar::comparison_operator::less_equal,
                            context_.create<tscalar::variable_reference>(expr.target()->region(), v_target),
                            context_.create<tscalar::variable_reference>(expr.right()->region(), v_right)));
            auto backward = context_.create<tscalar::binary>(
                    expr.region(),
                    tscalar::binary_operator::conditional_and,
                    context_.create<tscalar::compare>(
                            expr.region(),
                            tscalar::comparison_operator::less_equal,
                            context_.create<tscalar::variable_reference>(expr.right()->region(), v_right),
                            context_.create<tscalar::variable_reference>(expr.target()->region(), v_target)),
                    context_.create<tscalar::compare>(
                            expr.region(),
                            tscalar::comparison_operator::less_equal,
                            context_.create<tscalar::variable_reference>(expr.target()->region(), v_target),
                            context_.create<tscalar::variable_reference>(expr.left()->region(), v_left)));
            auto body = context_.create<tscalar::binary>(
                    expr.operator_kind()->region(),
                    tscalar::binary_operator::conditional_or,
                    std::move(forward),
                    std::move(backward));
            result = context_.create<tscalar::let>(
                    expr.region(),
                    std::move(variables),
                    std::move(body));
        } else {
            auto v_target = factory_.stream_variable();
            std::vector<tscalar::let::variable> variables {};
            variables.reserve(1);
            variables.emplace_back(v_target, target.release());

            result = context_.create<tscalar::let>(
                    expr.region(),
                    std::move(variables),
                    context_.create<tscalar::binary>(
                            expr.region(),
                            tscalar::binary_operator::conditional_and,
                            context_.create<tscalar::compare>(
                                    expr.region(),
                                    tscalar::comparison_operator::less_equal,
                                    left.release(),
                                    context_.create<tscalar::variable_reference>(expr.target()->region(), v_target)),
                            context_.create<tscalar::compare>(
                                    expr.region(),
                                    tscalar::comparison_operator::less_equal,
                                    context_.create<tscalar::variable_reference>(expr.target()->region(), v_target),
                                    right.release())));
        }
        if (*expr.is_not()) {
            result = context_.create<tscalar::unary>(
                    expr.is_not().region(),
                    tscalar::unary_operator::conditional_not,
                    std::move(result));
        }
        return result;
    }

    // FIXME: impl in_predicate,
    // FIXME: impl pattern_match_predicate,
    // FIXME: impl table_predicate,
    // FIXME: impl <match predicate>
    // FIXME: impl <type predicate>
    // FIXME: impl function_invocation,

    [[nodiscard]] std::unique_ptr<tscalar::expression> operator()(
            ast::scalar::builtin_function_invocation const& expr,
            value_context const&) {
        using kind = ast::scalar::builtin_function_kind;
        switch (*expr.function()) {
            case kind::nullif:
                // FIXME: impl nullif (built-in function)
            case kind::coalesce:
                // FIXME: impl coalesce (built-in function)
            case kind::next_value_for:
                // FIXME: impl next_value_for (built-in function)

            case kind::convert:
            case kind::translate:
            case kind::current_path:
                context_.report(
                        sql_analyzer_code::unsupported_feature,
                        string_builder {}
                                << "unsupported special built-in function invocation: "
                                << expr.function()
                                << string_builder::to_string,
                        expr.function().region());
                return {};

            default:
                break;
        }

        auto function_name = to_string_view(*expr.function());
        ::takatori::util::reference_vector<tscalar::expression> arguments {};
        arguments.reserve(expr.arguments().size());
        std::vector<std::shared_ptr<ttype::data const>> argument_types {};
        argument_types.reserve(expr.arguments().size());
        for (auto&& arg : expr.arguments()) {
            auto value = process(*arg, {});
            if (!value) {
                return {};
            }
            auto type = context_.resolve(*value);
            if (!type) {
                return {};
            }
            arguments.push_back(value.release());
            argument_types.emplace_back(type);
        }

        auto&& path = context_.options()->schema_search_path();
        std::vector<std::shared_ptr<::yugawara::function::declaration const>> candidates {};
        for (auto&& schema : path.elements()) {
            // FIXME: consider override over schemas
            schema->function_provider().each(
                    function_name,
                    expr.arguments().size(),
                    [&](std::shared_ptr<::yugawara::function::declaration const> const& ptr) -> void {
                        if (is_applicable(argument_types, ptr->shared_parameter_types())) {
                            candidates.emplace_back(ptr);
                        }
                    });
        }

        if (candidates.empty()) {
            string_builder buffer {};
            buffer << "function not found: "
                   << expr.function();
            append_string(buffer, argument_types);
            context_.report(
                    sql_analyzer_code::function_not_found,
                    buffer << string_builder::to_string,
                    expr.region());
            return {};
        }
        auto target = resolve_overload(expr.function(), candidates);
        if (!target) {
            return {};
        }

        auto result = context_.create<tscalar::function_call>(
                expr.region(),
                factory_(std::move(target)),
                std::move(arguments));

        return result;
    }

    [[nodiscard]] std::unique_ptr<tscalar::expression> operator()(
            ast::scalar::builtin_set_function_invocation const& expr,
            value_context const&) {
        std::string function_name { to_string_view(*expr.function()) };
        if (expr.quantifier() == ast::scalar::set_quantifier::distinct) {
            function_name.append(yugawara::aggregate::declaration::name_suffix_distinct);
        }

        ::takatori::util::reference_vector<tscalar::expression> arguments {};
        arguments.reserve(expr.arguments().size());
        std::vector<std::shared_ptr<ttype::data const>> argument_types {};
        argument_types.reserve(expr.arguments().size());
        for (auto&& arg : expr.arguments()) {
            auto value = process(*arg, {});
            if (!value) {
                return {};
            }
            auto type = context_.resolve(*value);
            if (!type) {
                return {};
            }
            arguments.push_back(value.release());
            argument_types.emplace_back(type);
        }

        auto&& path = context_.options()->schema_search_path();
        std::vector<std::shared_ptr<::yugawara::aggregate::declaration const>> candidates {};
        for (auto&& schema : path.elements()) {
            // FIXME: consider override over schemas
            schema->set_function_provider().each(
                    function_name,
                    expr.arguments().size(),
                    [&](std::shared_ptr<::yugawara::aggregate::declaration const> const& ptr) -> void {
                        if (is_applicable(argument_types, ptr->shared_parameter_types())) {
                            candidates.emplace_back(ptr);
                        }
                    });
        }

        if (candidates.empty()) {
            string_builder buffer {};
            buffer << "set function not found: "
                   << expr.function();
            if (expr.quantifier()) {
                buffer << "[" << *expr.quantifier() << "]";
            }
            append_string(buffer, argument_types);
            context_.report(
                    sql_analyzer_code::function_not_found,
                    buffer << string_builder::to_string,
                    expr.region());
            return {};
        }
        auto target = resolve_overload(expr.function(), candidates);
        if (!target) {
            return {};
        }

        auto result = context_.create<::yugawara::extension::scalar::aggregate_function_call>(
                expr.region(),
                factory_(std::move(target)),
                std::move(arguments));

        saw_aggregate_ = true;
        return result;
    }

    [[nodiscard]] static bool is_applicable(
            std::vector<std::shared_ptr<::takatori::type::data const>> const& arguments,
            std::vector<std::shared_ptr<::takatori::type::data const>> const& parameters) {
        if (arguments.size() != parameters.size()) {
            return false;
        }
        for (std::size_t i = 0, n = arguments.size(); i < n; ++i) {
            auto&& arg = *arguments[i];
            auto&& param = *parameters[i];
            auto r = ::yugawara::type::is_parameter_application_convertible(arg, param);
            if (r != ::yugawara::util::ternary::yes) {
                return false;
            }
        }
        return true;
    }

    template<class TDecl, class TNode>
    [[nodiscard]] std::shared_ptr<TDecl> resolve_overload(
            ast::common::regioned<TNode> const& title,
            std::vector<std::shared_ptr<TDecl>> const& candidates) {
        std::size_t left_idx = 0;
        for (std::size_t right_idx = 1, n = candidates.size(); right_idx < n; ++right_idx) {
            auto const* left = candidates[left_idx].get();
            auto const* right = candidates[right_idx].get();
            auto forward_applicable = is_applicable(left->shared_parameter_types(), right->shared_parameter_types());
            auto reverse_applicable = is_applicable(right->shared_parameter_types(), left->shared_parameter_types());
            if (forward_applicable == reverse_applicable) {
                string_builder builder {};
                builder << "ambiguous function overload: "
                        << left->name()
                        << " - ";
                append_string(builder, left->shared_parameter_types());
                builder << " and ";
                append_string(builder, right->shared_parameter_types());
                context_.report(
                        sql_analyzer_code::function_ambiguous,
                        builder << string_builder::to_string,
                        title.region());
                return {};
            }
            if (reverse_applicable) {
                left_idx = right_idx;
            }
        }
        return candidates[left_idx];
    }

    static void append_string(
            string_builder& destination,
            std::vector<std::shared_ptr<ttype::data const>> const& types) {
        destination << "(";
        for (std::size_t i = 0, n = types.size(); i < n; ++i) {
            if (i != 0) {
                destination << ", ";
            }
            destination << *types[i];
        }
        destination << ")";
    }

    // FIXME: impl new_invocation,
    // FIXME: impl method_invocation,
    // FIXME: impl static_method_invocation,
    // FIXME: impl current_of_cursor,

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

    [[nodiscard]] bool validate(tscalar::expression const& expression) {
        if (context_.options()->validate_scalar_expressions()) {
            auto r = context_.resolve(expression, true);
            return static_cast<bool>(r);
        }
        return true;
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
