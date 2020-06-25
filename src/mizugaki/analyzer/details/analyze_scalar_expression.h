#pragma once

#include <takatori/scalar/expression.h>
#include <takatori/util/optional_ptr.h>

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

} // namespace mizugaki::analyzer::details
