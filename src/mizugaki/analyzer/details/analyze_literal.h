#pragma once

#include <takatori/scalar/expression.h>

#include <mizugaki/ast/literal/literal.h>
#include <mizugaki/analyzer/details/analyzer_context.h>
#include <mizugaki/analyzer/details/scalar_value_context.h>

namespace mizugaki::analyzer::details {

[[nodiscard]] std::unique_ptr<::takatori::scalar::expression> analyze_literal(
        analyzer_context& context,
        ast::literal::literal const& literal,
        scalar_value_context const& value_context = {});

} // namespace mizugaki::analyzer::details
