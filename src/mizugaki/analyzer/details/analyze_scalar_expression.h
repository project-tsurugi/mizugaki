#pragma once

#include <takatori/scalar/expression.h>

#include <yugawara/function/declaration.h>

#include <mizugaki/ast/scalar/expression.h>
#include <mizugaki/analyzer/details/analyzer_context.h>
#include <mizugaki/analyzer/details/query_scope.h>
#include <mizugaki/analyzer/details/value_context.h>

namespace mizugaki::analyzer::details {

class analyze_scalar_expression_result {
public:
    analyze_scalar_expression_result() = default;
    analyze_scalar_expression_result( // NOLINT: implicit conversion
            std::unique_ptr<::takatori::scalar::expression> expression,
            bool saw_aggregate = false) noexcept;

    [[nodiscard]] bool has_value() const noexcept;
    [[nodiscard]] explicit operator bool() const noexcept;
    [[nodiscard]] ::takatori::scalar::expression& value();
    [[nodiscard]] ::takatori::scalar::expression const& value() const;
    [[nodiscard]] ::takatori::scalar::expression& operator*();
    [[nodiscard]] ::takatori::scalar::expression const& operator*() const;
    [[nodiscard]] std::unique_ptr<::takatori::scalar::expression> release();
    [[nodiscard]] bool saw_aggregate() const noexcept;

private:
    std::unique_ptr<::takatori::scalar::expression> expression_ {};
    bool saw_aggregate_ {};
};

[[nodiscard]] analyze_scalar_expression_result analyze_scalar_expression(
        analyzer_context& context,
        ast::scalar::expression const& expression,
        query_scope const& scope,
        value_context const& value_context = {});

/**
 * @brief returns whether the given argument set are applicable to the parameter set.
 * @param arguments the argument types
 * @param parameters the parameter types
 * @return true if it is applicable
 * @return false otherwise
 */
[[nodiscard]] bool is_parameter_applicable(
        std::vector<std::shared_ptr<::takatori::type::data const>> const& arguments,
        std::vector<std::shared_ptr<::takatori::type::data const>> const& parameters);

/**
 * @brief resolves function overload from the given function list.
 * @param context the current analyzer context
 * @param functions the candidate function list
 * @param region the region for reporting
 * @return the resolved function declaration
 * @return empty if overload resolution was failed by ambiguity or not found
 */
[[nodiscard]] std::shared_ptr<::yugawara::function::declaration const> resolve_function_overload(
        analyzer_context& context,
        std::vector<std::shared_ptr<::yugawara::function::declaration const>> const& functions,
        ast::node_region region = {});

} // namespace mizugaki::analyzer::details
