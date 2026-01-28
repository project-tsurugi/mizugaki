#include "query_info.h"

namespace mizugaki::analyzer::details {

query_info::query_info(
        ::takatori::relation::graph_type query_graph,
        std::vector<::takatori::descriptor::variable> output_columns,
        std::vector<std::optional<std::string>> output_column_names) noexcept:
    query_graph_ { std::move(query_graph) },
    output_columns_ { std::move(output_columns) },
    output_column_names_ { std::move(output_column_names) }
{}

takatori::relation::graph_type& query_info::query_graph() noexcept {
    return query_graph_;
}

takatori::relation::graph_type const& query_info::query_graph() const noexcept {
    return query_graph_;
}

std::vector<takatori::descriptor::variable>& query_info::output_columns() noexcept {
    return output_columns_;
}

std::vector<takatori::descriptor::variable> const& query_info::output_columns() const noexcept {
    return output_columns_;
}

std::vector<std::optional<std::string>>& query_info::output_column_names() noexcept {
    return output_column_names_;
}

std::vector<std::optional<std::string>> const& query_info::output_column_names() const noexcept {
    return output_column_names_;
}

} // namespace mizugaki::analyzer::details
