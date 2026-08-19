#include <mizugaki/parser/sql_scanner.h>

#include <gtest/gtest.h>

namespace mizugaki::parser {

class sql_scanner_test : public ::testing::Test {};

using symbol_kind_type = sql_scanner::symbol_kind_type;

TEST_F(sql_scanner_test, is_contextual_keyword) {
    EXPECT_TRUE(is_contextual_keyword(symbol_kind_type::S_ASC));
    EXPECT_TRUE(is_contextual_keyword(symbol_kind_type::S_DESC));

    EXPECT_FALSE(is_contextual_keyword(symbol_kind_type::S_REGULAR_IDENTIFIER));
    EXPECT_FALSE(is_contextual_keyword(symbol_kind_type::S_DELIMITED_IDENTIFIER));

    EXPECT_FALSE(is_contextual_keyword(symbol_kind_type::S_SELECT));
    EXPECT_FALSE(is_contextual_keyword(symbol_kind_type::S_INSERT));
    EXPECT_FALSE(is_contextual_keyword(symbol_kind_type::S_UPDATE));
    EXPECT_FALSE(is_contextual_keyword(symbol_kind_type::S_DELETE));
}

TEST_F(sql_scanner_test, collapse_identifier_like_tokens_trivial) {
    auto result = collapse_identifier_like_tokens({});
    ASSERT_EQ(result.size(), 0);
}

TEST_F(sql_scanner_test, collapse_identifier_like_tokens_not_identifier) {
    std::vector<symbol_kind_type> candidates {
        symbol_kind_type::S_SELECT,
        symbol_kind_type::S_INSERT,
        symbol_kind_type::S_UPDATE,
        symbol_kind_type::S_DELETE,
    };
    auto result = collapse_identifier_like_tokens(candidates);
    ASSERT_EQ(result, candidates);
}

TEST_F(sql_scanner_test, collapse_identifier_like_tokens_contextual_keywords) {
    std::vector<symbol_kind_type> candidates {
        symbol_kind_type::S_ASC,
        symbol_kind_type::S_DESC,
        symbol_kind_type::S_COMMA,
    };
    auto result = collapse_identifier_like_tokens(candidates);
    ASSERT_EQ(result, candidates);
}

TEST_F(sql_scanner_test, collapse_identifier_like_tokens_regular_identifier) {
    std::vector<symbol_kind_type> candidates {
        symbol_kind_type::S_ASC,
        symbol_kind_type::S_DESC,
        symbol_kind_type::S_REGULAR_IDENTIFIER,
        symbol_kind_type::S_COMMA,
    };
    auto result = collapse_identifier_like_tokens(candidates);
    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], symbol_kind_type::S_REGULAR_IDENTIFIER);
    EXPECT_EQ(result[1], symbol_kind_type::S_COMMA);
}

TEST_F(sql_scanner_test, collapse_identifier_like_tokens_identifiers) {
    std::vector<symbol_kind_type> candidates {
        symbol_kind_type::S_ASC,
        symbol_kind_type::S_DESC,
        symbol_kind_type::S_REGULAR_IDENTIFIER,
        symbol_kind_type::S_DELIMITED_IDENTIFIER,
        symbol_kind_type::S_COMMA,
    };
    auto result = collapse_identifier_like_tokens(candidates);
    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], symbol_kind_type::S_REGULAR_IDENTIFIER);
    EXPECT_EQ(result[1], symbol_kind_type::S_COMMA);
}

} // namespace mizugaki::parser
