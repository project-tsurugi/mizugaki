#pragma once

#include <mizugaki/parser/sql_parser_options.h>

#include <mizugaki/analyzer/sql_analyzer_options.h>

#include <yugawara/compiler_options.h>

namespace mizugaki::examples::explain_cli {

[[nodiscard]] ::mizugaki::parser::sql_parser_options parser_options();

[[nodiscard]] ::mizugaki::analyzer::sql_analyzer_options analyzer_options();

[[nodiscard]] ::yugawara::compiler_options compiler_options();

} // namespace mizugaki::examples::explain_cli
