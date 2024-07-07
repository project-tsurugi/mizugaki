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

sql_scanner::location_type sql_scanner::location(bool eof) noexcept {
    return { cursor_ - (eof ? 0 : yyleng), cursor_ };
}

ast::common::chars sql_scanner::get_image(sql_driver const&) {
    std::string image {
            yytext,
            static_cast<std::size_t>(yyleng),
    };
    return image;
}

void sql_scanner::enter_comment() noexcept {
    comment_begin_ = cursor_ - yyleng;
}

sql_scanner::location_type sql_scanner::exit_comment(bool inclusive) noexcept {
    return {
            std::exchange(comment_begin_, npos),
            inclusive ? cursor_ : cursor_ - yyleng,
    };
}

} // namespace mizugaki::parser

