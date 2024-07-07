#include <mizugaki/parser/sql_parser.h>

#include <gtest/gtest.h>

#include <string>

#include <mizugaki/ast/statement/select_statement.h>

#include <mizugaki/ast/literal/string.h>

#include <mizugaki/ast/scalar/literal_expression.h>
#include <mizugaki/ast/scalar/value_constructor.h>

#include <mizugaki/ast/query/table_reference.h>
#include <mizugaki/ast/query/table_value_constructor.h>

#include "utils.h"

namespace mizugaki::parser {

using namespace testing;

using std::string_literals::operator""s;

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

TEST_F(sql_parser_misc_test, identifier_single_underscore) {
    sql_parser parser;
    auto result = parser("-", R"(TABLE _x;)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (query::table_reference {
            name::simple { "_x" },
    }));
}

TEST_F(sql_parser_misc_test, keyword_value) {
    sql_parser parser;
    auto result = parser("-", R"(TABLE VALUE;)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (query::table_reference {
            name::simple { "VALUE" },
    }));
}

TEST_F(sql_parser_misc_test, null_in_line_comment) {
    sql_parser parser;
    auto result = parser("-", "-- \0 NUL"s);
    ASSERT_TRUE(result) << diagnostics(result);

    auto&& unit = **result;
    ASSERT_EQ(unit.comments().size(), 1);
    EXPECT_EQ(comment(unit, 0), "-- \0 NUL"s);
}

TEST_F(sql_parser_misc_test, null_in_block_comment) {
    sql_parser parser;
    auto result = parser("-", "/* \0 NUL */"s);
    ASSERT_TRUE(result) << diagnostics(result);

    auto&& unit = **result;
    ASSERT_EQ(unit.comments().size(), 1);
    EXPECT_EQ(comment(unit, 0), "/* \0 NUL */"s);
}

TEST_F(sql_parser_misc_test, null_in_string_literal) {
    sql_parser parser;
    auto result = parser("-", "VALUES ('\0 NUL')"s);
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (query::table_value_constructor {
            scalar::value_constructor {
                    scalar::literal_expression {
                            literal::string { literal::kind::character_string, "'\0 NUL'"s },
                    },
            },
    }));
}

TEST_F(sql_parser_misc_test, null_in_delimited_identifier) {
    sql_parser parser;
    auto result = parser("-", "VALUES (\"\0 NUL\")"s);
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (query::table_value_constructor {
            scalar::value_constructor {
                    scalar::variable_reference {
                            name::simple { "\0 NUL"s, name::identifier_kind::delimited },
                    }
            },
    }));
}

} // namespace mizugaki::parser
