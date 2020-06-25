#pragma once

#include <memory>

#include <takatori/type/data.h>

#include <mizugaki/ast/type/type.h>
#include <mizugaki/analyzer/details/analyzer_context.h>
#include <mizugaki/analyzer/details/query_scope.h>

namespace mizugaki::analyzer::details {

/**
 * @brief analyzes the AST type.
 * @param context the current context
 * @param type the AST type to convert
 * @return the analyzed type
 * @return empty if error was occurred
 */
[[nodiscard]] std::shared_ptr<::takatori::type::data const> analyze_type(
        analyzer_context& context,
        ast::type::type const& type);

} // namespace mizugaki::analyzer::details
