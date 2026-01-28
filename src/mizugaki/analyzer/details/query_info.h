#pragma once

#include <optional>
#include <string>
#include <vector>

#include <takatori/descriptor/variable.h>

#include <takatori/relation/graph.h>

namespace mizugaki::analyzer::details {

/**
 * @brief holds information of a query structure (for subqueries).
 */
class query_info {
public:
    /**
     * @brief creates a new instance.
     * @param query_graph the query graph: it may not be completed and has just one output port without opposite
     * @param output_columns the output columns
     * @param output_column_names each column name of output_columns
     */
    query_info(
            ::takatori::relation::graph_type query_graph,
            std::vector<::takatori::descriptor::variable> output_columns,
            std::vector<std::optional<std::string>> output_column_names) noexcept;

    /**
     * @brief returns the query graph.
     * @return the query graph
     */
    [[nodiscard]] ::takatori::relation::graph_type& query_graph() noexcept;

    /// @copydoc query_graph()
    [[nodiscard]] ::takatori::relation::graph_type const& query_graph() const noexcept;

    /**
     * @brief returns the output columns.
     * @return the output columns
     */
    [[nodiscard]] std::vector<::takatori::descriptor::variable>& output_columns() noexcept;

    /// @copydoc output_columns()
    [[nodiscard]] std::vector<::takatori::descriptor::variable> const& output_columns() const noexcept;

    /**
     * @brief returns the output column names.
     * @return the output column names
     */
    [[nodiscard]] std::vector<std::optional<std::string>>& output_column_names() noexcept;

    /// @copydoc output_column_names()
    [[nodiscard]] std::vector<std::optional<std::string>> const& output_column_names() const noexcept;

private:
    ::takatori::relation::graph_type query_graph_;
    std::vector<::takatori::descriptor::variable> output_columns_;
    std::vector<std::optional<std::string>> output_column_names_;
};

} // namespace mizugaki::analyzer::details
