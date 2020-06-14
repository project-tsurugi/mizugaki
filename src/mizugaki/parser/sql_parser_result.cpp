#include <mizugaki/parser/sql_parser_result.h>

namespace mizugaki::parser {

sql_parser_result::sql_parser_result(value_type value) noexcept :
    value_ { std::move(value) }
{}

sql_parser_result::sql_parser_result(sql_parser_result::error_type error) noexcept :
    error_ { std::move(error) }
{}

sql_parser_result::sql_parser_result(location_type location, message_type message) noexcept :
    error_ {
            location,
            std::move(message),
    }
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

bool sql_parser_result::has_error() const noexcept {
    return !error_.second.empty();
}

sql_parser_result::error_type& sql_parser_result::error() noexcept {
    return error_;
}

sql_parser_result::error_type const& sql_parser_result::error() const noexcept {
    return error_;
}

} // namespace mizugaki::parser

