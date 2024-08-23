#include <mizugaki/parser/sql_parser_result.h>

namespace mizugaki::parser {

sql_parser_result::sql_parser_result(value_type value) noexcept :
    value_ { std::move(value) }
{}

sql_parser_result::sql_parser_result(sql_parser_result::diagnostic_type diagnostic) noexcept :
    diagnostic_ { std::move(diagnostic) }
{}

bool sql_parser_result::has_value() const noexcept {
    return value_ != nullptr;
}

sql_parser_result::operator bool() const noexcept {
    return has_value();
}

sql_parser_result::value_type& sql_parser_result::value() noexcept {
    return value_;
}

sql_parser_result::value_type const& sql_parser_result::value() const noexcept {
    return value_;
}

sql_parser_result::value_type& sql_parser_result::operator*() noexcept {
    return value();
}

sql_parser_result::value_type const& sql_parser_result::operator*() const noexcept {
    return value();
}

bool sql_parser_result::has_diagnostic() const noexcept {
    return static_cast<bool>(diagnostic_);
}

sql_parser_result::diagnostic_type& sql_parser_result::diagnostic() noexcept {
    return diagnostic_;
}

sql_parser_result::diagnostic_type const& sql_parser_result::diagnostic() const noexcept {
    return diagnostic_;
}

std::size_t &sql_parser_result::tree_node_count() noexcept {
    return tree_node_count_;
}

std::size_t sql_parser_result::tree_node_count() const noexcept {
    return tree_node_count_;
}

std::size_t &sql_parser_result::max_tree_depth() noexcept {
    return max_tree_depth_;
}

std::size_t sql_parser_result::max_tree_depth() const noexcept {
    return max_tree_depth_;
}

} // namespace mizugaki::parser

