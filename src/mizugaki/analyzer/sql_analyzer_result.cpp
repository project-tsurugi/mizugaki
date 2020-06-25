#include <mizugaki/analyzer/sql_analyzer_result.h>

namespace mizugaki::analyzer {

sql_analyzer_result::sql_analyzer_result(std::vector<diagnostic_type> element) noexcept
    : entity_(std::in_place_index<static_cast<std::size_t>(kind_type::diagnostics)>, std::move(element))
{}

sql_analyzer_result::sql_analyzer_result(std::unique_ptr<::takatori::relation::graph_type> element) noexcept
    : entity_(std::in_place_index<static_cast<std::size_t>(kind_type::execution_plan)>, std::move(element))
{}

sql_analyzer_result::sql_analyzer_result(std::unique_ptr<::takatori::statement::statement> element) noexcept
    : entity_(std::in_place_index<static_cast<std::size_t>(kind_type::statement)>, std::move(element))
{}

std::ostream& operator<<(std::ostream& out, sql_analyzer_result const& value) {
    return out << value.kind();
}

} // namespace mizugaki::analyzer
