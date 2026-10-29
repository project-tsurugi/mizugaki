#pragma once

#include <takatori/scalar/expression.h>
#include <takatori/relation/graph.h>
#include <takatori/util/optional_ptr.h>

#include <mizugaki/ast/query/expression.h>
#include <mizugaki/analyzer/details/analyzer_context.h>
#include <mizugaki/analyzer/details/query_scope.h>
#include <mizugaki/analyzer/details/relation_info.h>
#include <mizugaki/analyzer/details/row_value_context.h>

namespace mizugaki::analyzer::details {

class analyze_query_expression_result {
public:
    analyze_query_expression_result() = default;

    analyze_query_expression_result(
            ::takatori::relation::expression::output_port_type& output,
            relation_info relation,
            bool explicitly_ordered = false) noexcept;

    [[nodiscard]] bool has_value() const noexcept;
    [[nodiscard]] explicit operator bool() const noexcept;
    ::takatori::relation::expression::output_port_type& output();
    relation_info& relation() noexcept;

    /**
     * @brief returns whether or not the query expression explicitly requires the order of its result rows.
     * @details This is true only if the query expression itself has an ORDER BY clause
     *      (including the body of WITH clause), and does not consider whether or not the output
     *      relation is an ordered relation in the execution plan.
     *      That is, the ORDER BY clauses in the derived tables or subqueries never affect this.
     * @return true if the query expression explicitly requires the order of its result rows
     * @return false otherwise
     */
    [[nodiscard]] bool explicitly_ordered() const noexcept;

private:
    ::takatori::util::optional_ptr<::takatori::relation::expression::output_port_type> output_ {};
    relation_info relation_ {};
    bool explicitly_ordered_ { false };
};

[[nodiscard]] analyze_query_expression_result analyze_query_expression(
        analyzer_context& context,
        ::takatori::relation::graph_type& graph,
        ast::query::expression const& expression,
        ::takatori::util::optional_ptr<query_scope> parent,
        row_value_context const& value_context);

[[nodiscard]] relation_info build_relation_info(
        analyzer_context& context,
        ::yugawara::storage::table const& table,
        bool escape_columns = true,
        bool include_system_columns = false);

} // namespace mizugaki::analyzer::details
