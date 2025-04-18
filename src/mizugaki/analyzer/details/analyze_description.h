#pragma once

#include <string>

#include <takatori/document/document.h>

#include <mizugaki/analyzer/details/analyzer_context.h>

#include <mizugaki/ast/node_region.h>

namespace mizugaki::analyzer::details {

[[nodiscard]] std::string analyze_description(::takatori::document::document const& document, ast::node_region region);

[[nodiscard]] std::string analyze_description(analyzer_context const& context, ast::node_region region);

} // namespace mizugaki::analyzer::details
