#include <mizugaki/parser/sql_parser.h>

#include <gtest/gtest.h>

#include <string_view>

#include <mizugaki/ast/statement/select_statement.h>

#include <mizugaki/ast/query/table_reference.h>

#include "utils.h"

namespace mizugaki::parser {

using namespace testing;

using std::string_view_literals::operator""sv;

class sql_parser_error_test : public ::testing::Test {
protected:
    sql_parser_diagnostic parse_erroneous(std::string content) {
        sql_parser parser;
        auto result = parser("-", std::move(content));
        if (result.has_value()) {
            ADD_FAILURE() << *result.value();
            return {};
        }
        if (!result.has_diagnostic()) {
            ADD_FAILURE() << "no diagnostics";
            return {};
        }
        return std::move(result.diagnostic());
    }

    void print_result(sql_parser_diagnostic const& diagnostic) {
        auto&& document = *diagnostic.document();
        std::cout << "  source: " << document.contents(0, document.size()) << std::endl;
        std::cout << "    code: " << diagnostic.code() << std::endl;
        std::cout << " message: " << diagnostic.message() << std::endl;
        std::cout << "  region: " << diagnostic.region() << std::endl;
        std::cout << "contents: " << diagnostic.contents() << std::endl;
        std::cout << std::endl;
    }
};

TEST_F(sql_parser_error_test, invalid_character) {
    std::string content { R"(SELECT $1)" };
    auto r = parse_erroneous(content);
    ASSERT_TRUE(r);

    EXPECT_EQ(r.code(), sql_parser_code::invalid_character);
    EXPECT_NE(r.message().find(R"("$")"), std::string::npos) << r.message();
    EXPECT_EQ(r.contents(), "$");
    print_result(r);
}

TEST_F(sql_parser_error_test, invalid_character_not_ascii) {
    std::string content { u8"SELECT \u3042" };
    auto r = parse_erroneous(content);
    ASSERT_TRUE(r);

    EXPECT_EQ(r.code(), sql_parser_code::invalid_character);
    EXPECT_EQ(r.contents(), u8"\u3042"sv);
    print_result(r);
}

TEST_F(sql_parser_error_test, invalid_character_emoji) {
    std::string content { u8"SELECT \U0001F44D" };
    auto r = parse_erroneous(content);
    ASSERT_TRUE(r);

    EXPECT_EQ(r.code(), sql_parser_code::invalid_character);
    EXPECT_EQ(r.contents(), u8"\U0001F44D"sv);
    print_result(r);
}

TEST_F(sql_parser_error_test, invalid_character_control_control_00) {
    using std::string_literals::operator""s;
    auto content = "SELECT \0"s;
    auto r = parse_erroneous(content);
    ASSERT_TRUE(r);

    EXPECT_EQ(r.code(), sql_parser_code::invalid_character);
    EXPECT_NE(r.message().find("U+0000"), std::string::npos) << r.message();
    print_result(r);
}

TEST_F(sql_parser_error_test, invalid_character_control_control_1f) {
    using std::string_literals::operator""s;
    auto content = "SELECT \x1f"s;
    auto r = parse_erroneous(content);
    ASSERT_TRUE(r);

    EXPECT_EQ(r.code(), sql_parser_code::invalid_character);
    EXPECT_NE(r.message().find("U+001F"), std::string::npos) << r.message();
    print_result(r);
}

TEST_F(sql_parser_error_test, unexpected_eof_in_block_comments) {
    std::string content { R"(SELECT /* ... )" };
    auto r = parse_erroneous(content);
    ASSERT_TRUE(r);

    EXPECT_EQ(r.code(), sql_parser_code::unexpected_eof);
    EXPECT_EQ(r.contents(), "/*");
    print_result(r);
}

TEST_F(sql_parser_error_test, regular_identifier_restricted) {
    std::string content { R"(SELECT __x)" };
    auto r = parse_erroneous(content);
    ASSERT_TRUE(r);

    EXPECT_EQ(r.code(), sql_parser_code::invalid_token);
    EXPECT_EQ(r.contents(), "__x");
    print_result(r);
}

TEST_F(sql_parser_error_test, delimited_identifier_restricted) {
    std::string content { R"(SELECT "__x")" };
    auto r = parse_erroneous(content);
    EXPECT_EQ(r.contents(), R"("__x")");
    ASSERT_TRUE(r);

    EXPECT_EQ(r.code(), sql_parser_code::invalid_token);
    print_result(r);
}

TEST_F(sql_parser_error_test, unexpected_token) {
    std::string content { R"(SELECT 1 _ 2)" };
    auto r = parse_erroneous(content);
    ASSERT_TRUE(r);

    EXPECT_EQ(r.code(), sql_parser_code::unexpected_token);
    EXPECT_EQ(r.contents(), "_");
    print_result(r);
}

TEST_F(sql_parser_error_test, unexpected_eof_in_grammar) {
    std::string content { R"(SELECT * FROM)" };
    auto r = parse_erroneous(content);
    ASSERT_TRUE(r);

    EXPECT_EQ(r.code(), sql_parser_code::unexpected_eof);
    print_result(r);
}

TEST_F(sql_parser_error_test, unexpected_token_less_candidates) {
    std::string content { R"(VALUES 1)" };
    auto r = parse_erroneous(content);
    ASSERT_TRUE(r);

    EXPECT_EQ(r.code(), sql_parser_code::unexpected_token);
    print_result(r);
}

} // namespace mizugaki::parser
