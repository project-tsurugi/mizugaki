#include <mizugaki/parser/sql_parser.h>

#include <gtest/gtest.h>

#include <mizugaki/ast/statement/select_statement.h>

#include <mizugaki/ast/query/query.h>
#include <mizugaki/ast/query/table_value_constructor.h>
#include <mizugaki/ast/query/table_reference.h>

#include <mizugaki/ast/scalar/cast_expression.h>
#include <mizugaki/ast/scalar/extract_expression.h>
#include <mizugaki/ast/scalar/trim_expression.h>
#include <mizugaki/ast/scalar/builtin_function_invocation.h>
#include <mizugaki/ast/scalar/builtin_set_function_invocation.h>
#include <mizugaki/ast/scalar/function_invocation.h>
#include <mizugaki/ast/scalar/method_invocation.h>
#include <mizugaki/ast/scalar/static_method_invocation.h>
#include <mizugaki/ast/scalar/value_constructor.h>

#include <mizugaki/ast/type/user_defined.h>

#include <mizugaki/ast/name/qualified.h>

#include "utils.h"

namespace mizugaki::parser {

using namespace testing;

class sql_parser_function_test : public ::testing::Test {};

static sql_parser_result parse(std::string_view str, sql_parser parser = sql_parser {}) {
    using ::takatori::util::string_builder;
    return parser("-", string_builder {}
            << "VALUES (" << str << ");"
            << string_builder::to_string);
}

static scalar::expression const& extract(sql_parser::result_type const& result) {
    auto&& stmt = result.value()->statements().at(0);
    auto&& select = downcast<statement::select_statement>(*stmt);
    auto&& tv = downcast<query::table_value_constructor>(*select.expression());
    auto&& rv = downcast<scalar::value_constructor>(*tv.elements()[0]);
    return *rv.elements()[0];
}

TEST_F(sql_parser_function_test, position) {
    auto result = parse("POSITION(a IN b)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::builtin_function_invocation {
            scalar::builtin_function_kind::position,
            {
                    v("a"),
                    v("b"),
            },
    }));
}

TEST_F(sql_parser_function_test, char_length) {
    auto result = parse("CHAR_LENGTH(a)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::builtin_function_invocation {
            scalar::builtin_function_kind::character_length,
            {
                    v("a"),
            },
    }));
}

TEST_F(sql_parser_function_test, character_length) {
    auto result = parse("CHARACTER_LENGTH(a)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::builtin_function_invocation {
            scalar::builtin_function_kind::character_length,
            {
                    v("a"),
            },
    }));
}

TEST_F(sql_parser_function_test, octet_length) {
    auto result = parse("OCTET_LENGTH(a)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::builtin_function_invocation {
            scalar::builtin_function_kind::octet_length,
            {
                    v("a"),
            },
    }));
}

TEST_F(sql_parser_function_test, bit_length) {
    auto result = parse("BIT_LENGTH(a)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::builtin_function_invocation {
            scalar::builtin_function_kind::bit_length,
            {
                    v("a"),
            },
    }));
}

TEST_F(sql_parser_function_test, extention_length) {
    auto result = parse("LENGTH(a)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::builtin_function_invocation {
            scalar::builtin_function_kind::length,
            {
                    v("a"),
            },
    }));
}

TEST_F(sql_parser_function_test, extract_year) {
    auto result = parse("EXTRACT(YEAR FROM a)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::extract_expression {
            scalar::extract_field_kind::year,
            v("a"),
    }));
}

TEST_F(sql_parser_function_test, extract_month) {
    auto result = parse("EXTRACT(MONTH FROM a)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::extract_expression {
            scalar::extract_field_kind::month,
            v("a"),
    }));
}

TEST_F(sql_parser_function_test, extract_day) {
    auto result = parse("EXTRACT(DAY FROM a)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::extract_expression {
            scalar::extract_field_kind::day,
            v("a"),
    }));
}

TEST_F(sql_parser_function_test, extract_hour) {
    auto result = parse("EXTRACT(HOUR FROM a)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::extract_expression {
            scalar::extract_field_kind::hour,
            v("a"),
    }));
}

TEST_F(sql_parser_function_test, extract_minute) {
    auto result = parse("EXTRACT(MINUTE FROM a)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::extract_expression {
            scalar::extract_field_kind::minute,
            v("a"),
    }));
}

TEST_F(sql_parser_function_test, extract_second) {
    auto result = parse("EXTRACT(SECOND FROM a)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::extract_expression {
            scalar::extract_field_kind::second,
            v("a"),
    }));
}

TEST_F(sql_parser_function_test, extract_timezone_hour) {
    auto result = parse("EXTRACT(TIMEZONE_HOUR FROM a)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::extract_expression {
            scalar::extract_field_kind::timezone_hour,
            v("a"),
    }));
}

TEST_F(sql_parser_function_test, extract_timezone_minute) {
    auto result = parse("EXTRACT(TIMEZONE_MINUTE FROM a)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::extract_expression {
            scalar::extract_field_kind::timezone_minute,
            v("a"),
    }));
}

TEST_F(sql_parser_function_test, cardinality) {
    auto result = parse("CARDINALITY(a)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::builtin_function_invocation {
            scalar::builtin_function_kind::cardinality,
            {
                    v("a"),
            },
    }));
}

TEST_F(sql_parser_function_test, abs) {
    auto result = parse("ABS(a)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::builtin_function_invocation {
            scalar::builtin_function_kind::abs,
            {
                    v("a"),
            },
    }));
}

TEST_F(sql_parser_function_test, mod) {
    auto result = parse("MOD(a, b)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::builtin_function_invocation {
            scalar::builtin_function_kind::mod,
            {
                    v("a"),
                    v("b"),
            },
    }));
}

TEST_F(sql_parser_function_test, substring_arity2) {
    auto result = parse("SUBSTRING(a FROM b)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::builtin_function_invocation {
            scalar::builtin_function_kind::substring,
            {
                    v("a"),
                    v("b"),
            },
    }));
}

TEST_F(sql_parser_function_test, substring_arity3) {
    auto result = parse("SUBSTRING(a FROM b FOR c)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::builtin_function_invocation {
            scalar::builtin_function_kind::substring,
            {
                    v("a"),
                    v("b"),
                    v("c"),
            },
    }));
}

TEST_F(sql_parser_function_test, upper) {
    auto result = parse("UPPER(a)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::builtin_function_invocation {
            scalar::builtin_function_kind::upper,
            {
                    v("a"),
            },
    }));
}

TEST_F(sql_parser_function_test, lower) {
    auto result = parse("LOWER(a)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::builtin_function_invocation {
            scalar::builtin_function_kind::lower,
            {
                    v("a"),
            },
    }));
}

TEST_F(sql_parser_function_test, convert) {
    auto result = parse("CONVERT(a USING b)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::builtin_function_invocation {
            scalar::builtin_function_kind::convert,
            {
                    v("a"),
                    v("b"),
            },
    }));
}

TEST_F(sql_parser_function_test, trim) {
    auto result = parse("TRIM(a)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::trim_expression {
            {},
            {},
            v("a"),
    }));
}

TEST_F(sql_parser_function_test, trim_leading) {
    auto result = parse("TRIM(LEADING FROM a)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::trim_expression {
            scalar::trim_specification::leading,
            {},
            v("a"),
    }));
}

TEST_F(sql_parser_function_test, trim_trailing) {
    auto result = parse("TRIM(TRAILING FROM a)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::trim_expression {
            scalar::trim_specification::trailing,
            {},
            v("a"),
    }));
}

TEST_F(sql_parser_function_test, trim_both) {
    auto result = parse("TRIM(BOTH FROM a)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::trim_expression {
            scalar::trim_specification::both,
            {},
            v("a"),
    }));
}

TEST_F(sql_parser_function_test, trim_character) {
    auto result = parse("TRIM(c FROM a)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::trim_expression {
            {},
            v("c"),
            v("a"),
    }));
}

TEST_F(sql_parser_function_test, trim_all) {
    auto result = parse("TRIM(LEADING c FROM a)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::trim_expression {
            scalar::trim_specification::leading,
            v("c"),
            v("a"),
    }));
}

TEST_F(sql_parser_function_test, overlay_arity3) {
    auto result = parse("OVERLAY(a PLACING b FROM c)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::builtin_function_invocation {
            scalar::builtin_function_kind::overlay,
            {
                    v("a"),
                    v("b"),
                    v("c"),
            },
    }));
}

TEST_F(sql_parser_function_test, overlay_arity4) {
    auto result = parse("OVERLAY(a PLACING b FROM c FOR d)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::builtin_function_invocation {
            scalar::builtin_function_kind::overlay,
            {
                    v("a"),
                    v("b"),
                    v("c"),
                    v("d"),
            },
    }));
}

TEST_F(sql_parser_function_test, current_date) {
    auto result = parse("CURRENT_DATE");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::builtin_function_invocation {
            scalar::builtin_function_kind::current_date,
            {
            },
    }));
}

TEST_F(sql_parser_function_test, current_time) {
    auto result = parse("CURRENT_TIME");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::builtin_function_invocation {
            scalar::builtin_function_kind::current_time,
            {
            },
    }));
}

TEST_F(sql_parser_function_test, current_time_precision) {
    auto result = parse("CURRENT_TIME(10)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::builtin_function_invocation {
            scalar::builtin_function_kind::current_time,
            {
                int_literal("10"),
            },
    }));
}

TEST_F(sql_parser_function_test, localtime) {
    auto result = parse("LOCALTIME");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::builtin_function_invocation {
            scalar::builtin_function_kind::localtime,
            {
            },
    }));
}

TEST_F(sql_parser_function_test, current_timestamp) {
    auto result = parse("CURRENT_TIMESTAMP");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::builtin_function_invocation {
            scalar::builtin_function_kind::current_timestamp,
            {
            },
    }));
}

TEST_F(sql_parser_function_test, localtimestamp) {
    auto result = parse("LOCALTIMESTAMP");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::builtin_function_invocation {
            scalar::builtin_function_kind::localtimestamp,
            {
            },
    }));
}

TEST_F(sql_parser_function_test, count_asterisk) {
    auto result = parse("COUNT(*)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::builtin_set_function_invocation {
            scalar::builtin_set_function_kind::count,
            {},
            {},
    }));
}

TEST_F(sql_parser_function_test, count_element) {
    auto result = parse("COUNT(a)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::builtin_set_function_invocation {
            scalar::builtin_set_function_kind::count,
            {},
            {
                    v("a")
            },
    }));
}

TEST_F(sql_parser_function_test, count_all_element) {
    auto result = parse("COUNT(ALL a)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::builtin_set_function_invocation {
            scalar::builtin_set_function_kind::count,
            scalar::set_quantifier::all,
            {
                    v("a")
            },
    }));
}

TEST_F(sql_parser_function_test, count_distinct_element) {
    auto result = parse("COUNT(DISTINCT a)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::builtin_set_function_invocation {
            scalar::builtin_set_function_kind::count,
            scalar::set_quantifier::distinct,
            {
                    v("a")
            },
    }));
}

TEST_F(sql_parser_function_test, avg) {
    auto result = parse("AVG(a)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::builtin_set_function_invocation {
            scalar::builtin_set_function_kind::avg,
            {},
            {
                    v("a")
            },
    }));
}

TEST_F(sql_parser_function_test, max) {
    auto result = parse("MAX(a)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::builtin_set_function_invocation {
            scalar::builtin_set_function_kind::max,
            {},
            {
                    v("a")
            },
    }));
}

TEST_F(sql_parser_function_test, min) {
    auto result = parse("MIN(a)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::builtin_set_function_invocation {
            scalar::builtin_set_function_kind::min,
            {},
            {
                    v("a")
            },
    }));
}

TEST_F(sql_parser_function_test, sum) {
    auto result = parse("SUM(a)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::builtin_set_function_invocation {
            scalar::builtin_set_function_kind::sum,
            {},
            {
                    v("a")
            },
    }));
}

TEST_F(sql_parser_function_test, every) {
    auto result = parse("EVERY(a)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::builtin_set_function_invocation {
            scalar::builtin_set_function_kind::every,
            {},
            {
                    v("a")
            },
    }));
}

TEST_F(sql_parser_function_test, bool_and) {
    auto result = parse("BOOL_AND(a)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::builtin_set_function_invocation {
            scalar::builtin_set_function_kind::bool_and,
            {},
            {
                    v("a")
            },
    }));
}

TEST_F(sql_parser_function_test, bool_or) {
    auto result = parse("BOOL_OR(a)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::builtin_set_function_invocation {
            scalar::builtin_set_function_kind::bool_or,
            {},
            {
                    v("a")
            },
    }));
}

TEST_F(sql_parser_function_test, user_defined_function) {
    auto result = parse("f(a)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::function_invocation {
            name::simple { "f" },
            {
                    v("a")
            },
    }));
}

TEST_F(sql_parser_function_test, user_defined_function_identifier_chain) {
    auto result = parse("u.v.w(a)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::function_invocation {
            name::qualified {
                    name::simple { "u" },
                    name::simple { "v" },
                    name::simple { "w" },
            },
            {
                    v("a")
            },
    }));
}

TEST_F(sql_parser_function_test, user_defined_function_empty_arguments) {
    auto result = parse("f()");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::function_invocation {
            name::simple { "f" },
            {
            },
    }));
}

TEST_F(sql_parser_function_test, user_defined_function_multiple_arguments) {
    auto result = parse("f(a, b, c)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::function_invocation {
            name::simple { "f" },
            {
                    v("a"),
                    v("b"),
                    v("c"),
            },
    }));
}

TEST_F(sql_parser_function_test, user_defined_function_generalize_arguments) {
    auto result = parse("f(a AS t)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::function_invocation {
            name::simple { "f" },
            {
                    scalar::cast_expression {
                            scalar::cast_operator::generalize,
                            v("a"),
                            type::user_defined {
                                    name::simple { "t" },
                            }
                    },
            },
    }));
}

TEST_F(sql_parser_function_test, method_invocation) {
    auto result = parse("CAST(a AS t).f(b)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::method_invocation {
            scalar::cast_expression {
                    scalar::cast_operator::cast,
                    v("a"),
                    type::user_defined {
                            name::simple { "t" },
                    }
            },
            name::simple { "f" },
            {
                    v("b"),
            },
    }));
}

TEST_F(sql_parser_function_test, method_invocation_generalize) {
    auto result = parse("(a AS t).f(b)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::method_invocation {
            scalar::cast_expression {
                    scalar::cast_operator::generalize,
                    v("a"),
                    type::user_defined {
                            name::simple { "t" },
                    }
            },
            name::simple { "f" },
            {
                    v("b"),
            },
    }));
}

TEST_F(sql_parser_function_test, method_invocation_arguments) {
    auto result = parse("(a AS t).f(b, c, d)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::method_invocation {
            scalar::cast_expression {
                    scalar::cast_operator::generalize,
                    v("a"),
                    type::user_defined {
                            name::simple { "t" },
                    }
            },
            name::simple { "f" },
            {
                    v("b"),
                    v("c"),
                    v("d"),
            },
    }));
}

TEST_F(sql_parser_function_test, method_invocation_reference) {
    auto result = parse("CAST(a AS t)->f(b)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::method_invocation {
            scalar::cast_expression {
                    scalar::cast_operator::cast,
                    v("a"),
                    type::user_defined {
                            name::simple { "t" },
                    }
            },
            name::simple { "f" },
            {
                    v("b"),
            },
            scalar::reference_operator::arrow,
    }));
}

TEST_F(sql_parser_function_test, static_method_invocation) {
    auto result = parse("t::f(a)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::static_method_invocation {
            type::user_defined {
                    name::simple { "t" },
            },
            name::simple { "f" },
            {
                    v("a")
            },
    }));
}

TEST_F(sql_parser_function_test, static_method_invocation_identifier_chain) {
    auto result = parse("u.v.w::f(a)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::static_method_invocation {
            type::user_defined {
                    name::qualified {
                            name::simple { "u" },
                            name::simple { "v" },
                            name::simple { "w" },
                    },
            },
            name::simple { "f" },
            {
                    v("a")
            },
    }));
}

TEST_F(sql_parser_function_test, static_method_invocation_multiple_arguments) {
    auto result = parse("t::f(a, b, c)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::static_method_invocation {
            type::user_defined {
                    name::simple { "t" },
            },
            name::simple { "f" },
            {
                    v("a"),
                    v("b"),
                    v("c"),
            },
    }));
}

} // namespace mizugaki::parser
