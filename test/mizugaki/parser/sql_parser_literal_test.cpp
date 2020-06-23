#include <mizugaki/parser/sql_parser.h>

#include <gtest/gtest.h>

#include <mizugaki/ast/statement/select_statement.h>

#include <mizugaki/ast/query/query.h>
#include <mizugaki/ast/query/table_value_constructor.h>
#include <mizugaki/ast/scalar/value_constructor.h>

#include <mizugaki/ast/literal/boolean.h>
#include <mizugaki/ast/literal/numeric.h>
#include <mizugaki/ast/literal/string.h> // NOLINT
#include <mizugaki/ast/literal/datetime.h>
#include <mizugaki/ast/literal/interval.h>
#include <mizugaki/ast/literal/special.h>

#include "utils.h"

namespace mizugaki::parser {

using namespace testing;

class sql_parser_literal_test : public ::testing::Test {};

static sql_parser_result parse(std::string_view str, sql_parser parser = sql_parser {}) {
    using ::takatori::util::string_builder;
    return parser("-", string_builder {}
            << "VALUES (" << str << ");"
            << string_builder::to_string);
}

static literal::literal const& extract(sql_parser::result_type const& result) {
    auto&& stmt = result.value()->statements().at(0);
    auto&& select = downcast<statement::select_statement>(*stmt);
    auto&& tv = downcast<query::table_value_constructor>(*select.expression());
    auto&& rv = downcast<scalar::value_constructor>(*tv.elements()[0]);
    auto&& le = downcast<scalar::literal_expression>(*rv.elements()[0]);
    return *le.value();
}

TEST_F(sql_parser_literal_test, character_string) {
    auto result = parse("'Hello, world!'");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (literal::string {
            literal::kind::character_string,
            "'Hello, world!'",
    }));
}

TEST_F(sql_parser_literal_test, character_string_concatenations) {
    auto result = parse("'Hello' 'World' '!'");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (literal::string {
            literal::kind::character_string,
            "'Hello'",
            "'World'",
            "'!'",
    }));
}

TEST_F(sql_parser_literal_test, bit_string) {
    auto result = parse("B'0100'");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (literal::string {
            literal::kind::bit_string,
            "'0100'",
    }));
}

TEST_F(sql_parser_literal_test, hex_string) {
    auto result = parse("X'DEADbeef'");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (literal::string {
            literal::kind::hex_string,
            "'DEADbeef'",
    }));
}

TEST_F(sql_parser_literal_test, unsigned_integer) {
    auto result = parse("100");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (literal::numeric {
            literal::kind::exact_numeric,
            "100",
    }));
}

TEST_F(sql_parser_literal_test, exact_numeric) {
    auto result = parse("3.141592");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (literal::numeric {
            literal::kind::exact_numeric,
            "3.141592",
    }));
}

TEST_F(sql_parser_literal_test, approximate_numeric) {
    auto result = parse("6.02E+23");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (literal::numeric {
            literal::kind::approximate_numeric,
            "6.02E+23",
    }));
}

TEST_F(sql_parser_literal_test, date) {
    auto result = parse("DATE '2020-06-24'");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (literal::datetime {
            literal::kind::date,
            "'2020-06-24'",
    }));
}

TEST_F(sql_parser_literal_test, time) {
    auto result = parse("TIME '01:23:45.678'");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (literal::datetime {
            literal::kind::time,
            "'01:23:45.678'",
    }));
}

TEST_F(sql_parser_literal_test, timestamp) {
    auto result = parse("TIMESTAMP '2020-01-02 34:56:78'");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (literal::datetime {
            literal::kind::timestamp,
            "'2020-01-02 34:56:78'",
    }));
}

TEST_F(sql_parser_literal_test, interval) {
    auto result = parse("INTERVAL '1'");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (literal::interval {
            "'1'",
    }));
}

TEST_F(sql_parser_literal_test, interval_plus) {
    auto result = parse("INTERVAL + '1'");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (literal::interval {
            ast::literal::sign::plus,
            "'1'",
    }));
}

TEST_F(sql_parser_literal_test, interval_minus) {
    auto result = parse("INTERVAL - '1'");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (literal::interval {
            ast::literal::sign::minus,
            "'1'",
    }));
}

TEST_F(sql_parser_literal_test, boolean_true) {
    auto result = parse("TRUE");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (literal::boolean {
            ast::literal::boolean_kind::true_,
    }));
}

TEST_F(sql_parser_literal_test, boolean_false) {
    auto result = parse("FALSE");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (literal::boolean {
            ast::literal::boolean_kind::false_,
    }));
}

TEST_F(sql_parser_literal_test, boolean_unknown) {
    auto result = parse("UNKNOWN");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (literal::boolean {
            ast::literal::boolean_kind::unknown,
    }));
}

TEST_F(sql_parser_literal_test, contextual_null) {
    auto result = parse("NULL");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (literal::null {}));
}

TEST_F(sql_parser_literal_test, contextual_empty) {
    auto result = parse("ARRAY[]");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (literal::empty {}));
}

TEST_F(sql_parser_literal_test, contextual_default) {
    auto result = parse("DEFAULT");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (literal::default_ {}));
}

} // namespace mizugaki::parser
