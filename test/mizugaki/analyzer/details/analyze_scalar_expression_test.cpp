#include <mizugaki/analyzer/details/analyze_scalar_expression.h>

#include <gtest/gtest.h>

#include <takatori/value/primitive.h>
#include <takatori/type/primitive.h>
#include <takatori/type/decimal.h>

#include <takatori/scalar/binary.h>
#include <takatori/scalar/cast.h>
#include <takatori/scalar/compare.h>
#include <takatori/scalar/unary.h>

#include <yugawara/binding/factory.h>

#include <yugawara/aggregate/declaration.h>

#include <yugawara/extension/scalar/aggregate_function_call.h>

#include <mizugaki/ast/type/simple.h>

#include <mizugaki/ast/literal/boolean.h>
#include <mizugaki/ast/literal/special.h>

#include <mizugaki/ast/scalar/binary_expression.h>
#include <mizugaki/ast/scalar/builtin_set_function_invocation.h>
#include <mizugaki/ast/scalar/cast_expression.h>
#include <mizugaki/ast/scalar/comparison_predicate.h>
#include <mizugaki/ast/scalar/host_parameter_reference.h>
#include <mizugaki/ast/scalar/unary_expression.h>

#include "test_parent.h"

namespace mizugaki::analyzer::details {

using namespace ::mizugaki::analyzer::testing;

class analyze_scalar_expression_test : public test_parent {
protected:
    ::takatori::descriptor::aggregate_function descriptor(std::shared_ptr<::yugawara::aggregate::declaration const> decl) {
        return ::yugawara::binding::factory {}(std::move(decl));
    }
};

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

TEST_F(analyze_scalar_expression_test, host_parameter_reference_value_with_colon) {
    options_.host_parameter_declaration_starts_with_colon() = true;
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

TEST_F(analyze_scalar_expression_test, host_parameter_reference_variable_with_colon) {
    options_.host_parameter_declaration_starts_with_colon() = true;
    auto decl = host_parameters_.add({ ":x", ttype::int4 {} });
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::host_parameter_reference { id(":x") },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    EXPECT_EQ(*r, vref(::yugawara::binding::factory {}(decl)));
}

TEST_F(analyze_scalar_expression_test, host_parameter_reference_value_without_colon) {
    options_.host_parameter_declaration_starts_with_colon() = false;
    auto c0 = vdesc();
    placeholders_.add("x", { ttype::int8 {}, tvalue::int8 { 1 } });

    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::host_parameter_reference { id(":x") },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    EXPECT_EQ(*r, immediate(1));
}

TEST_F(analyze_scalar_expression_test, host_parameter_reference_variable_without_colon) {
    options_.host_parameter_declaration_starts_with_colon() = false;
    auto decl = host_parameters_.add({ "x", ttype::int4 {} });
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
            ast::scalar::host_parameter_reference { id("x") },
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

TEST_F(analyze_scalar_expression_test, builtin_set_function_invocation_simple) {
    auto func = set_functions_->add(::yugawara::aggregate::declaration {
            ::yugawara::aggregate::declaration::minimum_builtin_function_id + 1,
            "count",
            ttype::int8 {},
            {},
            true,
    });

    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::builtin_set_function_invocation {
                    ast::scalar::builtin_set_function_kind::count,
                    {},
                    {},
            },
            {},
            {});
    ASSERT_TRUE(r);
    EXPECT_TRUE(r.saw_aggregate());
    EXPECT_EQ(*r, (::yugawara::extension::scalar::aggregate_function_call {
            descriptor(func),
            {},
    }));
}

TEST_F(analyze_scalar_expression_test, builtin_set_function_invocation_args) {
    auto empty = set_functions_->add(::yugawara::aggregate::declaration {
            ::yugawara::aggregate::declaration::minimum_builtin_function_id + 1,
            "count",
            ttype::int8 {},
            {},
            true,
    });
    auto args = set_functions_->add(::yugawara::aggregate::declaration {
            ::yugawara::aggregate::declaration::minimum_builtin_function_id + 2,
            "count",
            ttype::int8 {},
            {
                    ttype::int8 {},
            },
            true,
    });

    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::builtin_set_function_invocation {
                    ast::scalar::builtin_set_function_kind::count,
                    ast::scalar::set_quantifier::all,
                    {
                            literal(number("1")),
                    },
            },
            {},
            {});
    ASSERT_TRUE(r);
    EXPECT_TRUE(r.saw_aggregate());
    EXPECT_EQ(*r, (::yugawara::extension::scalar::aggregate_function_call {
            descriptor(args),
            {
                    immediate(1),
            },
    }));
}

TEST_F(analyze_scalar_expression_test, builtin_set_function_invocation_distinct) {
    auto all = set_functions_->add(::yugawara::aggregate::declaration {
            ::yugawara::aggregate::declaration::minimum_builtin_function_id + 1,
            "sum",
            ttype::int8 {},
            {
                    ttype::int8 {},
            },
            true,
    });
    auto distinct = set_functions_->add(::yugawara::aggregate::declaration {
            ::yugawara::aggregate::declaration::minimum_builtin_function_id + 2,
            std::string { "sum" }.append(::yugawara::aggregate::declaration::name_suffix_distinct),
            ttype::int8 {},
            {
                    ttype::int8 {},
            },
            true,
    });

    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::builtin_set_function_invocation {
                    ast::scalar::builtin_set_function_kind::sum,
                    ast::scalar::set_quantifier::distinct,
                    {
                            literal(number("1")),
                    },
            },
            {},
            {});
    ASSERT_TRUE(r);
    EXPECT_TRUE(r.saw_aggregate());
    EXPECT_EQ(*r, (::yugawara::extension::scalar::aggregate_function_call {
            descriptor(distinct),
            {
                    immediate(1),
            },
    }));
}

TEST_F(analyze_scalar_expression_test, builtin_set_function_invocation_overload) {
    auto f_int4 = set_functions_->add(::yugawara::aggregate::declaration {
            ::yugawara::aggregate::declaration::minimum_builtin_function_id + 1,
            "max",
            ttype::int8 {},
            {
                    ttype::int4 {},
            },
            true,
    });
    auto f_int8 = set_functions_->add(::yugawara::aggregate::declaration {
            ::yugawara::aggregate::declaration::minimum_builtin_function_id + 2,
            "max",
            ttype::int8 {},
            {
                    ttype::int8 {},
            },
            true,
    });
    auto f_decimal = set_functions_->add(::yugawara::aggregate::declaration {
            ::yugawara::aggregate::declaration::minimum_builtin_function_id + 3,
            "max",
            ttype::decimal {},
            {
                    ttype::decimal {},
            },
            true,
    });

    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::builtin_set_function_invocation {
                    ast::scalar::builtin_set_function_kind::max,
                    ast::scalar::set_quantifier::all,
                    {
                            literal(number("1")),
                    },
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    EXPECT_TRUE(r.saw_aggregate());
    EXPECT_EQ(*r, (::yugawara::extension::scalar::aggregate_function_call {
            descriptor(f_int8),
            {
                    immediate(1),
            },
    }));
}

TEST_F(analyze_scalar_expression_test, builtin_set_function_invocation_missing) {
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::builtin_set_function_invocation {
                    ast::scalar::builtin_set_function_kind::count,
                    {},
                    {},
            },
            {},
            {});
    EXPECT_FALSE(r);
    EXPECT_TRUE(find_error(diagnostic_code::function_not_found));
}

TEST_F(analyze_scalar_expression_test, builtin_set_function_invocation_ambiguous) {
    set_functions_->add(::yugawara::aggregate::declaration {
            ::yugawara::aggregate::declaration::minimum_builtin_function_id + 1,
            "count",
            ttype::int8 {},
            {},
            true,
    });
    set_functions_->add(::yugawara::aggregate::declaration {
            ::yugawara::aggregate::declaration::minimum_builtin_function_id + 2,
            "count",
            ttype::int8 {},
            {},
            true,
    });
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::builtin_set_function_invocation {
                    ast::scalar::builtin_set_function_kind::count,
                    {},
                    {},
            },
            {},
            {});
    EXPECT_FALSE(r);
    EXPECT_TRUE(find_error(diagnostic_code::function_ambiguous));
}

} // namespace mizugaki::analyzer::details
