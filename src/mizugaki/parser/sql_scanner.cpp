#include <mizugaki/parser/sql_scanner.h>

#include <utility>

#include <cstdlib>

namespace mizugaki::parser {

sql_scanner::sql_scanner(std::istream& input) :
    super { std::addressof(input) }
{}

void sql_scanner::LexerError(char const* msg) {
    // FIXME: impl
    super::LexerError(msg);
}

void sql_scanner::user_action() noexcept {
    cursor_ += yyleng;
}

sql_scanner::location_type sql_scanner::location() noexcept {
    return { cursor_ - yyleng, cursor_ };
}

ast::common::chars sql_scanner::get_image(sql_driver const& driver) {
    return {
            YYText(),
            driver.get_object_creator().allocator(),
    };
}

void sql_scanner::enter_comment() noexcept {
    comment_begin_ = cursor_ - yyleng;
}

sql_scanner::location_type sql_scanner::exit_comment() noexcept {
    return {
            std::exchange(comment_begin_, npos),
            cursor_,
    };
}

} // namespace mizugaki::parser

