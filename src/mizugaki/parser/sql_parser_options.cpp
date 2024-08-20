#include <mizugaki/parser/sql_parser_options.h>

namespace mizugaki::parser {

sql_parser_options::sql_parser_options() :
    element_limits_ { std::make_unique<sql_parser_element_map<size_type>>() }
{

}

sql_parser_options::size_type& sql_parser_options::max_expected_candidates() noexcept {
    return max_expected_candidates_;
}

sql_parser_options::size_type const& sql_parser_options::max_expected_candidates() const noexcept {
    return max_expected_candidates_;
}

sql_parser_element_map<sql_parser_options::size_type>& sql_parser_options::element_limits() noexcept {
    return *element_limits_;
}

sql_parser_element_map<sql_parser_options::size_type> const& sql_parser_options::element_limits() const noexcept {
    return *element_limits_;
}

int& sql_parser_options::debug() noexcept {
    return debug_;
}

int const& sql_parser_options::debug() const noexcept {
    return debug_;
}

} // namespace mizugaki::parser