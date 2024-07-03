#include <mizugaki/parser/sql_parser.h>

#include <gtest/gtest.h>

#include <mizugaki/ast/statement/select_statement.h>

#include <mizugaki/ast/query/table_value_constructor.h>
#include <mizugaki/ast/query/table_reference.h>

#include <mizugaki/ast/scalar/literal_expression.h>
#include <mizugaki/ast/scalar/variable_reference.h>
#include <mizugaki/ast/scalar/host_parameter_reference.h>
#include <mizugaki/ast/scalar/field_reference.h>
#include <mizugaki/ast/scalar/case_expression.h>
#include <mizugaki/ast/scalar/cast_expression.h>
#include <mizugaki/ast/scalar/new_invocation.h>
#include <mizugaki/ast/scalar/unary_expression.h>
#include <mizugaki/ast/scalar/binary_expression.h>
#include <mizugaki/ast/scalar/value_constructor.h>
#include <mizugaki/ast/scalar/builtin_function_invocation.h>
#include <mizugaki/ast/scalar/subquery.h>

#include <mizugaki/ast/type/simple.h>
#include <mizugaki/ast/type/user_defined.h>

#include <mizugaki/ast/name/simple.h>
#include <mizugaki/ast/name/qualified.h>

#include "utils.h"

namespace mizugaki::parser {

using namespace testing;

class sql_parser_scalar_test : public ::testing::Test {};

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
    auto&& sv = rv.elements()[0];
    if (!sv) {
        throw std::domain_error("expression is null");
    }
    return *sv;
}

TEST_F(sql_parser_scalar_test, literal_expression) {
    auto result = parse("1");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), int_literal("1"));
}

TEST_F(sql_parser_scalar_test, variable_reference) {
    auto result = parse("v");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::variable_reference {
            name::simple { "v" },
    }));
}

TEST_F(sql_parser_scalar_test, variable_reference_delimited) {
    auto result = parse(R"("Hello, ""world""!")");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::variable_reference {
            name::simple { R"(Hello, "world"!)" }.delimited(),
    }));
}

TEST_F(sql_parser_scalar_test, variable_reference_identifier_chain) {
    auto result = parse("v.w.x");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::variable_reference {
            name::qualified {
                    name::simple { "v" },
                    name::simple { "w" },
                    name::simple { "x" },
            },
    }));
}

TEST_F(sql_parser_scalar_test, host_parameter_reference) {
    auto result = parse(":v");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::host_parameter_reference {
            name::simple { ":v" },
    }));
}

TEST_F(sql_parser_scalar_test, field_reference) {
    auto result = parse("CAST(x AS t).f");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::field_reference {
            scalar::cast_expression {
                    scalar::cast_operator::cast,
                    v("x"),
                    type::user_defined {
                            name::simple { "t" },
                    },
            },
            name::simple { "f" },
    }));
}

TEST_F(sql_parser_scalar_test, element_reference) {
    auto result = parse("a[i]");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::binary_expression {
            v("a"),
            scalar::binary_operator::element_reference,
            v("i"),
    }));
}

TEST_F(sql_parser_scalar_test, dereference_operator) {
    auto result = parse("r->f");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::field_reference {
            v("r"),
            name::simple { "f" },
            scalar::reference_operator::arrow,
    }));
}

TEST_F(sql_parser_scalar_test, reference_resolution) {
    auto result = parse("DEREF(r)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::unary_expression {
            scalar::unary_operator::reference_resolution,
            v("r"),
    }));
}

TEST_F(sql_parser_scalar_test, case_expression_nullif) {
    auto result = parse("NULLIF(a, b)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::builtin_function_invocation {
            scalar::builtin_function_kind::nullif,
            {
                    v("a"),
                    v("b"),
            },
    }));
}

TEST_F(sql_parser_scalar_test, case_expression_coalesce) {
    auto result = parse("COALESCE(a, b)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::builtin_function_invocation {
            scalar::builtin_function_kind::coalesce,
            {
                    v("a"),
                    v("b"),
            },
    }));
}

TEST_F(sql_parser_scalar_test, case_expression_coalesce_multiple) {
    auto result = parse("COALESCE(a, b, c, d, e)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::builtin_function_invocation {
            scalar::builtin_function_kind::coalesce,
            {
                    v("a"),
                    v("b"),
                    v("c"),
                    v("d"),
                    v("e"),
            },
    }));
}

TEST_F(sql_parser_scalar_test, case_expression_simple) {
    auto result = parse(
            "CASE x "
            "  WHEN a0 THEN b0 "
            "  WHEN a1 THEN b1 "
            "END");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::case_expression {
            v("x"),
            {
                    { v("a0"), v("b0") },
                    { v("a1"), v("b1") },
            }
    }));
}

TEST_F(sql_parser_scalar_test, case_expression_simple_multiple) {
    auto result = parse(
            "CASE x "
            "  WHEN a0 THEN b0 "
            "  WHEN a1 THEN b1 "
            "  WHEN a2 THEN b2 "
            "  WHEN a3 THEN b3 "
            "END");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::case_expression {
            v("x"),
            {
                    { v("a0"), v("b0") },
                    { v("a1"), v("b1") },
                    { v("a2"), v("b2") },
                    { v("a3"), v("b3") },
            }
    }));
}

TEST_F(sql_parser_scalar_test, case_expression_simple_else) {
    auto result = parse(
            "CASE x "
            "  WHEN a0 THEN b0 "
            "  ELSE b1 "
            "END");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::case_expression {
            v("x"),
            {
                    { v("a0"), v("b0") },
            },
            v("b1"),
    }));
}

TEST_F(sql_parser_scalar_test, case_expression_searched) {
    auto result = parse(
            "CASE "
            "  WHEN a0 THEN b0 "
            "  WHEN a1 THEN b1 "
            "END");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::case_expression {
            {
                    { v("a0"), v("b0") },
                    { v("a1"), v("b1") },
            }
    }));
}

TEST_F(sql_parser_scalar_test, case_expression_searched_multiple) {
    auto result = parse(
            "CASE "
            "  WHEN a0 THEN b0 "
            "  WHEN a1 THEN b1 "
            "  WHEN a2 THEN b2 "
            "  WHEN a3 THEN b3 "
            "END");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::case_expression {
            {
                    { v("a0"), v("b0") },
                    { v("a1"), v("b1") },
                    { v("a2"), v("b2") },
                    { v("a3"), v("b3") },
            }
    }));
}

TEST_F(sql_parser_scalar_test, case_expression_searched_else) {
    auto result = parse(
            "CASE "
            "  WHEN a0 THEN b0 "
            "  ELSE b1 "
            "END");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::case_expression {
            {
                    { v("a0"), v("b0") },
            },
            v("b1"),
    }));
}

TEST_F(sql_parser_scalar_test, cast_specification) {
    auto result = parse("CAST(x AS t)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::cast_expression {
            scalar::cast_operator::cast,
            v("x"),
            type::user_defined {
                    name::simple { "t" },
            },
    }));
}

TEST_F(sql_parser_scalar_test, new_specification) {
    auto result = parse("NEW t(a, b, c)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::new_invocation {
            type::user_defined {
                    name::simple { "t" },
            },
            {
                    v("a"),
                    v("b"),
                    v("c"),
            },
    }));
}

TEST_F(sql_parser_scalar_test, generalized_expression) {
    auto result = parse("(x AS t)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::cast_expression {
            scalar::cast_operator::generalize,
            v("x"),
            type::user_defined {
                    name::simple { "t" },
            },
    }));
}

TEST_F(sql_parser_scalar_test, subtype_treatment) {
    auto result = parse("TREAT(x AS t)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::cast_expression {
            scalar::cast_operator::treat,
            v("x"),
            type::user_defined {
                    name::simple { "t" },
            },
    }));
}

TEST_F(sql_parser_scalar_test, binary_expression_plus) {
    auto result = parse("a + b");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::binary_expression {
            v("a"),
            scalar::binary_operator::plus,
            v("b"),
    }));
}

TEST_F(sql_parser_scalar_test, binary_expression_minus) {
    auto result = parse("a - b");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::binary_expression {
            v("a"),
            scalar::binary_operator::minus,
            v("b"),
    }));
}

TEST_F(sql_parser_scalar_test, binary_expression_asterisk) {
    auto result = parse("a * b");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::binary_expression {
            v("a"),
            scalar::binary_operator::asterisk,
            v("b"),
    }));
}

TEST_F(sql_parser_scalar_test, binary_expression_solidus) {
    auto result = parse("a / b");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::binary_expression {
            v("a"),
            scalar::binary_operator::solidus,
            v("b"),
    }));
}

TEST_F(sql_parser_scalar_test, binary_expression_percent) {
    auto result = parse("a % b");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::binary_expression {
            v("a"),
            scalar::binary_operator::percent,
            v("b"),
    }));
}

TEST_F(sql_parser_scalar_test, binary_expression_concatenation) {
    auto result = parse("a || b");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::binary_expression {
            v("a"),
            scalar::binary_operator::concatenation,
            v("b"),
    }));
}

TEST_F(sql_parser_scalar_test, binary_expression_at_time_zone) {
    auto result = parse("a at time zone b");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::binary_expression {
            v("a"),
            scalar::binary_operator::at_time_zone,
            v("b"),
    }));
}

TEST_F(sql_parser_scalar_test, binary_expression_at_time_zone_plus) {
    auto result = parse("a at time zone +b");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::binary_expression {
            v("a"),
            scalar::binary_operator::at_time_zone,
            scalar::unary_expression {
                    scalar::unary_operator::plus,
                    v("b"),
            },
    }));
}

TEST_F(sql_parser_scalar_test, binary_expression_at_time_zone_minus) {
    auto result = parse("a at time zone -b");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::binary_expression {
            v("a"),
            scalar::binary_operator::at_time_zone,
            scalar::unary_expression {
                    scalar::unary_operator::minus,
                    v("b"),
            },
    }));
}

TEST_F(sql_parser_scalar_test, unary_expression_plus) {
    auto result = parse("+a");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::unary_expression {
            scalar::unary_operator::plus,
            v("a"),
    }));
}

TEST_F(sql_parser_scalar_test, unary_expression_minus) {
    auto result = parse("-a");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::unary_expression {
            scalar::unary_operator::minus,
            v("a"),
    }));
}

TEST_F(sql_parser_scalar_test, unary_expression_plus_literal) {
    auto result = parse("+1");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::literal_expression {
            ast::literal::numeric {
                    ast::literal::kind::exact_numeric,
                    ast::literal::sign::plus,
                    { "1" },
            },
    }));
}

TEST_F(sql_parser_scalar_test, unary_expression_minus_literal) {
    auto result = parse("-1");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::literal_expression {
            ast::literal::numeric {
                    ast::literal::kind::exact_numeric,
                    ast::literal::sign::minus,
                    { "1" },
            },
    }));
}

TEST_F(sql_parser_scalar_test, unary_expression_at_local) {
    auto result = parse("a at local");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::unary_expression {
            scalar::unary_operator::at_local,
            v("a"),
    }));
}

TEST_F(sql_parser_scalar_test, array_value_constructor) {
    auto result = parse("ARRAY [ a ]");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::value_constructor {
            scalar::value_constructor_kind::array,
            {
                    v("a"),
            },
    }));
}

TEST_F(sql_parser_scalar_test, array_value_constructor_multiple) {
    auto result = parse("ARRAY [ a, b, c ]");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::value_constructor {
            scalar::value_constructor_kind::array,
            {
                    v("a"),
                    v("b"),
                    v("c"),
            },
    }));
}

TEST_F(sql_parser_scalar_test, row_value_constructor) {
    auto result = parse("ROW (a)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::value_constructor {
            scalar::value_constructor_kind::row,
            {
                    v("a"),
            },
    }));
}

TEST_F(sql_parser_scalar_test, row_value_constructor_multiple) {
    auto result = parse("ROW (a, b, c)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::value_constructor {
            scalar::value_constructor_kind::row,
            {
                    v("a"),
                    v("b"),
                    v("c"),
            },
    }));
}

TEST_F(sql_parser_scalar_test, row_value_constructor_implicit) {
    auto result = parse("(a)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), v("a"));
}

TEST_F(sql_parser_scalar_test, row_value_constructor_implicit_multiple) {
    auto result = parse("(a, b, c)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::value_constructor {
            scalar::value_constructor_kind::row,
            {
                    v("a"),
                    v("b"),
                    v("c"),
            },
    }));
}

TEST_F(sql_parser_scalar_test, subquery) {
    auto result = parse("(TABLE a)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::subquery {
            query::table_reference {
                    name::simple { "a" },
            },
    }));
}

} // namespace mizugaki::parser
