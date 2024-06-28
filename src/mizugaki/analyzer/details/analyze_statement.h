#pragma once

#include <variant>

#include <takatori/relation/graph.h>
#include <takatori/statement/statement.h>

#include <mizugaki/ast/statement/statement.h>
#include <mizugaki/analyzer/details/analyzer_context.h>

namespace mizugaki::analyzer::details {

using execution_plan_result_type = std::unique_ptr<::takatori::relation::graph_type>;
using erroneous_result_type = std::nullptr_t;
using statement_result_type = std::unique_ptr<::takatori::statement::statement>;
using analyze_statement_result_type = std::variant<
        erroneous_result_type,
        execution_plan_result_type,
        statement_result_type>;

[[nodiscard]] analyze_statement_result_type analyze_statement(
        analyzer_context& context,
        ast::statement::statement const& statement);

} // namespace mizugaki::analyzer::details
