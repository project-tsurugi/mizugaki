#pragma once

#include <memory>
#include <string>

#include <mizugaki/parser/sql_parser_options.h>

#include <mizugaki/analyzer/sql_analyzer_options.h>

#include <yugawara/compiler_options.h>

#include <yugawara/schema/declaration.h>

namespace mizugaki::examples::explain_cli {

[[nodiscard]] std::shared_ptr<::yugawara::schema::declaration> create_default_schema(std::string name);

[[nodiscard]] parser::sql_parser_options parser_options();

[[nodiscard]] analyzer::sql_analyzer_options analyzer_options(
        std::shared_ptr<::yugawara::schema::declaration> default_schema);

[[nodiscard]] ::yugawara::compiler_options compiler_options();

} // namespace mizugaki::examples::explain_cli
