#include <mizugaki/parser/sql_parser.h>

#include <gtest/gtest.h>

#include <mizugaki/ast/statement/select_statement.h>

#include <mizugaki/ast/query/table_reference.h>

#include "utils.h"

namespace mizugaki::parser {

using namespace testing;

class sql_parser_misc_test : public ::testing::Test {};

static std::string_view comment(ast::compilation_unit const& unit, std::size_t index) {
    auto r = unit.comments().at(index);
    return unit.document()->contents(r.first(), r.size());
}

static query::expression const& extract(sql_parser::result_type const& result) {
    auto&& stmt = *result.value()->statements().at(0);
    auto&& select = downcast<statement::select_statement>(stmt);
    return *select.expression();
}

TEST_F(sql_parser_misc_test, simple_comment) {
    sql_parser parser;
    auto result = parser("-",
R"(
-- begin
;
-- middle
;
-- end)"
    );
    ASSERT_TRUE(result) << diagnostics(result);

    auto&& unit = **result;
    ASSERT_EQ(unit.comments().size(), 3);
    EXPECT_EQ(comment(unit, 0), "-- begin");
    EXPECT_EQ(comment(unit, 1), "-- middle");
    EXPECT_EQ(comment(unit, 2), "-- end");
}

TEST_F(sql_parser_misc_test, simple_comment_lf) {
    sql_parser parser;
    auto result = parser("-", "--a\n--b");
    ASSERT_TRUE(result) << diagnostics(result);

    auto&& unit = **result;
    ASSERT_EQ(unit.comments().size(), 2);
    EXPECT_EQ(comment(unit, 0), "--a");
    EXPECT_EQ(comment(unit, 1), "--b");
}

TEST_F(sql_parser_misc_test, simple_comment_crlf) {
    sql_parser parser;
    auto result = parser("-", "--a\r\n--b");
    ASSERT_TRUE(result) << diagnostics(result);

    auto&& unit = **result;
    ASSERT_EQ(unit.comments().size(), 2);
    EXPECT_EQ(comment(unit, 0), "--a");
    EXPECT_EQ(comment(unit, 1), "--b");
}

TEST_F(sql_parser_misc_test, block_comment) {
    sql_parser parser;
    auto result = parser("-",
R"(
/* begin */
;
/* middle */
;/* inline */;
/* end */)"
    );
    ASSERT_TRUE(result) << diagnostics(result);

    auto&& unit = **result;
    ASSERT_EQ(unit.comments().size(), 4);
    EXPECT_EQ(comment(unit, 0), "/* begin */");
    EXPECT_EQ(comment(unit, 1), "/* middle */");
    EXPECT_EQ(comment(unit, 2), "/* inline */");
    EXPECT_EQ(comment(unit, 3), "/* end */");
}

TEST_F(sql_parser_misc_test, block_comment_multiline) {
    sql_parser parser;
    auto result = parser("-",
R"(
/**
 * @brief test of multiline block comment.
 */
;
)"
    );
    ASSERT_TRUE(result) << diagnostics(result);

    auto&& unit = **result;
    ASSERT_EQ(unit.comments().size(), 1);
    EXPECT_EQ(comment(unit, 0), R"(/**
 * @brief test of multiline block comment.
 */)");
}

TEST_F(sql_parser_misc_test, block_comment_unclosed) {
    sql_parser parser;
    auto result = parser("-", "; /* unexpected EOF");
    EXPECT_FALSE(result);
}

TEST_F(sql_parser_misc_test, delimited_identifier) {
    sql_parser parser;
    auto result = parser("-", R"(TABLE "TABLE";)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (query::table_reference {
            name::simple { "TABLE" }.delimited(),
    }));
}

} // namespace mizugaki::parser
