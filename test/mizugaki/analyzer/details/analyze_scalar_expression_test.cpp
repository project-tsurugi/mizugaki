#include <mizugaki/analyzer/details/analyze_scalar_expression.h>

#include <gtest/gtest.h>

#include <takatori/value/primitive.h>
#include <takatori/type/primitive.h>

#include <takatori/scalar/binary.h>
#include <takatori/scalar/cast.h>
#include <takatori/scalar/compare.h>
#include <takatori/scalar/unary.h>

#include <yugawara/binding/factory.h>

#include <mizugaki/ast/type/simple.h>

#include <mizugaki/ast/literal/boolean.h>
#include <mizugaki/ast/literal/special.h>

#include <mizugaki/ast/scalar/binary_expression.h>
#include <mizugaki/ast/scalar/cast_expression.h>
#include <mizugaki/ast/scalar/comparison_predicate.h>
#include <mizugaki/ast/scalar/host_parameter_reference.h>
#include <mizugaki/ast/scalar/unary_expression.h>

#include "test_parent.h"

namespace mizugaki::analyzer::details {

using namespace ::mizugaki::analyzer::testing;

class analyze_scalar_expression_test : public test_parent {};

TEST_F(analyze_scalar_expression_test, literal_expression) {
    auto r = analyze_scalar_expression(
            context(),
            literal(number("1")),
            {},
            {});
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, immediate(1));
}

TEST_F(analyze_scalar_expression_test, variable_reference) {
    auto c0 = vdesc();
    query_scope scope {};
    auto&& rinfo = scope.add({});
    rinfo.add({ {}, c0, "x" });

    auto r = analyze_scalar_expression(
            context(),
            vref(id("x")),
            scope,
            {});
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, vref(c0));
}

TEST_F(analyze_scalar_expression_test, host_parameter_reference_value) {
    auto c0 = vdesc();
    placeholders_.add(":x", { ttype::int8 {}, tvalue::int8 { 1 } });

    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::host_parameter_reference { id(":x") },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    EXPECT_EQ(*r, immediate(1));
}

TEST_F(analyze_scalar_expression_test, host_parameter_reference_variable) {
    auto decl = host_variables_.add({ ":x", ttype::int4 {} });
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::host_parameter_reference { id(":x") },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    EXPECT_EQ(*r, vref(::yugawara::binding::factory {}(decl)));
}

TEST_F(analyze_scalar_expression_test, host_parameter_not_found) {
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::host_parameter_reference { id(":x") },
            {},
            {});
    EXPECT_FALSE(r);
    EXPECT_TRUE(find_error(diagnostic_code::variable_not_found));
}

TEST_F(analyze_scalar_expression_test, cast_expression) {
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::cast_expression {
                    ast::scalar::cast_operator::cast,
                    literal(number("1")),
                    ast::type::simple { ast::type::kind::tiny_integer },
            },
            {},
            {});
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, (tscalar::cast {
            ttype::int1 {},
            tscalar::cast::loss_policy_type::ignore,
            immediate(1),
    }));
}

TEST_F(analyze_scalar_expression_test, unary_expression_plus) {
    auto from = ast::scalar::unary_operator::plus;
    auto to = tscalar::unary_operator::plus;
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::unary_expression {
                    from,
                    literal(number("1")),
            },
            {},
            {});
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, (tscalar::unary {
            to,
            immediate(1),
    }));
}

TEST_F(analyze_scalar_expression_test, unary_expression_minus) {
    auto from = ast::scalar::unary_operator::minus;
    auto to = tscalar::unary_operator::sign_inversion;
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::unary_expression {
                    from,
                    literal(number("1")),
            },
            {},
            {});
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, (tscalar::unary {
            to,
            immediate(1),
    }));
}

TEST_F(analyze_scalar_expression_test, unary_expression_not) {
    auto from = ast::scalar::unary_operator::not_;
    auto to = tscalar::unary_operator::conditional_not;
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::unary_expression {
                    from,
                    literal(ast::literal::boolean { true }),
            },
            {},
            {});
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, (tscalar::unary {
            to,
            immediate_bool(true),
    }));
}

TEST_F(analyze_scalar_expression_test, binary_expression_plus) {
    auto from = ast::scalar::binary_operator::plus;
    auto to = tscalar::binary_operator::add;
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::binary_expression {
                    literal(number("1")),
                    from,
                    literal(number("2")),
            },
            {},
            {});
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, (tscalar::binary {
            to,
            immediate(1),
            immediate(2),
    }));
}

TEST_F(analyze_scalar_expression_test, binary_expression_minus) {
    auto from = ast::scalar::binary_operator::minus;
    auto to = tscalar::binary_operator::subtract;
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::binary_expression {
                    literal(number("1")),
                    from,
                    literal(number("2")),
            },
            {},
            {});
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, (tscalar::binary {
            to,
            immediate(1),
            immediate(2),
    }));
}

TEST_F(analyze_scalar_expression_test, binary_expression_asterisk) {
    auto from = ast::scalar::binary_operator::asterisk;
    auto to = tscalar::binary_operator::multiply;
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::binary_expression {
                    literal(number("1")),
                    from,
                    literal(number("2")),
            },
            {},
            {});
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, (tscalar::binary {
            to,
            immediate(1),
            immediate(2),
    }));
}

TEST_F(analyze_scalar_expression_test, binary_expression_solidus) {
    auto from = ast::scalar::binary_operator::solidus;
    auto to = tscalar::binary_operator::divide;
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::binary_expression {
                    literal(number("1")),
                    from,
                    literal(number("2")),
            },
            {},
            {});
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, (tscalar::binary {
            to,
            immediate(1),
            immediate(2),
    }));
}

TEST_F(analyze_scalar_expression_test, binary_expression_concatenation) {
    auto from = ast::scalar::binary_operator::concatenation;
    auto to = tscalar::binary_operator::concat;
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::binary_expression {
                    literal(string("'a'")),
                    from,
                    literal(string("'b'")),
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    EXPECT_EQ(*r, (tscalar::binary {
            to,
            immediate("a"),
            immediate("b"),
    }));
}

TEST_F(analyze_scalar_expression_test, binary_expression_and) {
    auto from = ast::scalar::binary_operator::and_;
    auto to = tscalar::binary_operator::conditional_and;
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::binary_expression {
                    literal(ast::literal::boolean { true }),
                    from,
                    literal(ast::literal::boolean { false }),
            },
            {},
            {});
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, (tscalar::binary {
            to,
            immediate_bool(true),
            immediate_bool(false),
    }));
}

TEST_F(analyze_scalar_expression_test, binary_expression_or) {
    auto from = ast::scalar::binary_operator::or_;
    auto to = tscalar::binary_operator::conditional_or;
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::binary_expression {
                    literal(ast::literal::boolean { true }),
                    from,
                    literal(ast::literal::boolean { false }),
            },
            {},
            {});
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, (tscalar::binary {
            to,
            immediate_bool(true),
            immediate_bool(false),
    }));
}

TEST_F(analyze_scalar_expression_test, binary_expression_is_null) {
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::binary_expression {
                    literal(ast::literal::boolean { true }),
                    ast::scalar::binary_operator::is,
                    literal(ast::literal::null {}),
            },
            {},
            {});
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, (tscalar::unary {
            tscalar::unary_operator::is_null,
            immediate_bool(true),
    }));
}

TEST_F(analyze_scalar_expression_test, binary_expression_is_true) {
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::binary_expression {
                    literal(ast::literal::boolean { true }),
                    ast::scalar::binary_operator::is,
                    literal(ast::literal::boolean { true }),
            },
            {},
            {});
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, (tscalar::unary {
            tscalar::unary_operator::is_true,
            immediate_bool(true),
    }));
}

TEST_F(analyze_scalar_expression_test, binary_expression_is_false) {
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::binary_expression {
                    literal(ast::literal::boolean { true }),
                    ast::scalar::binary_operator::is,
                    literal(ast::literal::boolean { false }),
            },
            {},
            {});
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, (tscalar::unary {
            tscalar::unary_operator::is_false,
            immediate_bool(true),
    }));
}

TEST_F(analyze_scalar_expression_test, binary_expression_is_unknown) {
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::binary_expression {
                    literal(ast::literal::boolean { true }),
                    ast::scalar::binary_operator::is,
                    literal(ast::literal::boolean { ast::literal::boolean_kind::unknown }),
            },
            {},
            {});
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, (tscalar::unary {
            tscalar::unary_operator::is_unknown,
            immediate_bool(true),
    }));
}

TEST_F(analyze_scalar_expression_test, binary_expression_is_invalid) {
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::binary_expression {
                    literal(ast::literal::boolean { true }),
                    ast::scalar::binary_operator::is,
                    literal(string("'INVALID'")),
            },
            {},
            {});
    EXPECT_FALSE(r);
    EXPECT_TRUE(find_error(diagnostic_code::malformed_syntax));
}

TEST_F(analyze_scalar_expression_test, binary_expression_is_not_null) {
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::binary_expression {
                    literal(ast::literal::boolean { true }),
                    ast::scalar::binary_operator::is_not,
                    literal(ast::literal::null {}),
            },
            {},
            {});
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, (tscalar::unary {
            tscalar::unary_operator::conditional_not,
            tscalar::unary {
                    tscalar::unary_operator::is_null,
                    immediate_bool(true),
            }
    }));
}

TEST_F(analyze_scalar_expression_test, comparison_predicate_equals) {
    auto from = ast::scalar::comparison_operator::equals;
    auto to = tscalar::comparison_operator::equal;
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::comparison_predicate {
                    literal(number("1")),
                    from,
                    literal(number("2")),
            },
            {},
            {});
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, (tscalar::compare {
            to,
            immediate(1),
            immediate(2),
    }));
}

TEST_F(analyze_scalar_expression_test, comparison_predicate_not_equals) {
    auto from = ast::scalar::comparison_operator::not_equals;
    auto to = tscalar::comparison_operator::not_equal;
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::comparison_predicate {
                    literal(number("1")),
                    from,
                    literal(number("2")),
            },
            {},
            {});
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, (tscalar::compare {
            to,
            immediate(1),
            immediate(2),
    }));
}

TEST_F(analyze_scalar_expression_test, comparison_predicate_less_than) {
    auto from = ast::scalar::comparison_operator::less_than;
    auto to = tscalar::comparison_operator::less;
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::comparison_predicate {
                    literal(number("1")),
                    from,
                    literal(number("2")),
            },
            {},
            {});
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, (tscalar::compare {
            to,
            immediate(1),
            immediate(2),
    }));
}

TEST_F(analyze_scalar_expression_test, comparison_predicate_greater_than) {
    auto from = ast::scalar::comparison_operator::greater_than;
    auto to = tscalar::comparison_operator::greater;
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::comparison_predicate {
                    literal(number("1")),
                    from,
                    literal(number("2")),
            },
            {},
            {});
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, (tscalar::compare {
            to,
            immediate(1),
            immediate(2),
    }));
}

TEST_F(analyze_scalar_expression_test, comparison_predicate_less_than_or_equals) {
    auto from = ast::scalar::comparison_operator::less_than_or_equals;
    auto to = tscalar::comparison_operator::less_equal;
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::comparison_predicate {
                    literal(number("1")),
                    from,
                    literal(number("2")),
            },
            {},
            {});
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, (tscalar::compare {
            to,
            immediate(1),
            immediate(2),
    }));
}

TEST_F(analyze_scalar_expression_test, comparison_predicate_greater_than_or_equals) {
    auto from = ast::scalar::comparison_operator::greater_than_or_equals;
    auto to = tscalar::comparison_operator::greater_equal;
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::comparison_predicate {
                    literal(number("1")),
                    from,
                    literal(number("2")),
            },
            {},
            {});
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, (tscalar::compare {
            to,
            immediate(1),
            immediate(2),
    }));
}

} // namespace mizugaki::analyzer::details
