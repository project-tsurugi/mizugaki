#include <mizugaki/parser/sql_scanner.h>

#include <algorithm>
#include <array>
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

void sql_scanner::on_token(::mizugaki::parser::sql_driver& driver, bool eof) {
    driver.add_comment_separator(location(eof));
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

bool is_contextual_keyword(sql_scanner::symbol_kind_type kind) noexcept {
    using symbol_kind_type = sql_scanner::symbol_kind_type;
    static constexpr std::array<symbol_kind_type, 14> contextual_keywords {
        symbol_kind_type::S_ASC,
        symbol_kind_type::S_CONTINUE,
        symbol_kind_type::S_DESC,
        symbol_kind_type::S_KEY,
        symbol_kind_type::S_IGNORE,
        symbol_kind_type::S_SCHEMA,
        symbol_kind_type::S_SEQUENCE,
        symbol_kind_type::S_RESTART,
        symbol_kind_type::S_RESTRICT,
        symbol_kind_type::S_RENAME,
        symbol_kind_type::S_FIRST,
        symbol_kind_type::S_LAST,
        symbol_kind_type::S_ORDINALITY,
        symbol_kind_type::S_VALUE,
    };
    auto iter = std::find(contextual_keywords.begin(), contextual_keywords.end(), kind);
    return iter != contextual_keywords.end();
}

std::vector<sql_scanner::symbol_kind_type> collapse_identifier_like_tokens(
        std::vector<sql_scanner::symbol_kind_type> candidates) {
    using symbol_kind_type = sql_scanner::symbol_kind_type;
    bool identifier_expected = std::any_of(candidates.begin(), candidates.end(), [](symbol_kind_type k) {
        return k == symbol_kind_type::S_REGULAR_IDENTIFIER
            || k == symbol_kind_type::S_DELIMITED_IDENTIFIER;
    });
    if (!identifier_expected) {
        return candidates;
    }
    std::vector<symbol_kind_type> result;
    result.reserve(candidates.size());
    result.push_back(symbol_kind_type::S_REGULAR_IDENTIFIER); // always prefer regular identifier over contextual keywords
    for (auto kind : candidates) {
        // skip identifier-like tokens
        if (kind != symbol_kind_type::S_REGULAR_IDENTIFIER
                && kind != symbol_kind_type::S_DELIMITED_IDENTIFIER
                && !is_contextual_keyword(kind)) {
            result.push_back(kind);
        }
    }
    return result;
}

} // namespace mizugaki::parser

