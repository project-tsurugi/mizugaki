#pragma once

// manually include flex system header
#if !defined(FLEX_SCANNER)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage): Flex framework matter
#define yyFlexLexer _generated_mizugaki__parser__sql_scanner_FlexLexer
#include <FlexLexer.h>
#undef yyFlexLexer
#undef yyFlexLexerOnce

#endif // !defined(FLEX_SCANNER)

#include <vector>

#include <mizugaki/ast/common/chars.h>

#include <mizugaki/parser/sql_driver.h>
#include <mizugaki/parser/sql_parser_generated.hpp>

namespace mizugaki::parser {

class sql_scanner : public _generated_mizugaki__parser__sql_scanner_FlexLexer {

    using super = _generated_mizugaki__parser__sql_scanner_FlexLexer;

public:
    using parser_type = sql_parser_generated;
    using value_type = parser_type::symbol_type;
    using location_type = sql_driver::location_type;
    using symbol_kind_type = parser_type::symbol_kind::symbol_kind_type;

    explicit sql_scanner(std::istream& input);

    [[nodiscard]] value_type next_token(::mizugaki::parser::sql_driver& driver);

protected:
    void LexerError(char const* msg) override;

private:
    std::size_t npos = static_cast<std::size_t>(-1);

    std::size_t cursor_ {};
    std::size_t comment_begin_ { npos };

    void on_token(::mizugaki::parser::sql_driver& driver, bool eof = false);

    void user_action() noexcept;
    [[nodiscard]] location_type location(bool eof = false) noexcept;

    void enter_comment() noexcept;
    [[nodiscard]] location_type exit_comment(bool inclusive) noexcept;

    ast::common::chars get_image(sql_driver const& driver);
};

[[nodiscard]] bool is_contextual_keyword(sql_scanner::symbol_kind_type kind) noexcept;

[[nodiscard]] std::vector<sql_scanner::symbol_kind_type> collapse_identifier_like_tokens(
        std::vector<sql_scanner::symbol_kind_type> candidates);

} // namespace sandbox
