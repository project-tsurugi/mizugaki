#include <mizugaki/analyzer/details/analyze_scalar_expression.h>

#include <gtest/gtest.h>

#include <takatori/type/primitive.h>

#include <takatori/scalar/binary.h>
#include <takatori/scalar/compare.h>
#include <takatori/scalar/conditional.h>
#include <takatori/scalar/let.h>
#include <takatori/scalar/unary.h>

#include <mizugaki/ast/scalar/binary_expression.h>
#include <mizugaki/ast/scalar/builtin_function_invocation.h>
#include <mizugaki/ast/scalar/comparison_predicate.h>
#include <mizugaki/ast/scalar/case_expression.h>

#include "test_parent.h"

namespace mizugaki::analyzer::details {

using namespace ::mizugaki::analyzer::testing;

class analyze_scalar_expression_case_test : public test_parent {
protected:
    void invalid(ast::scalar::expression const& expression) {
        auto r = analyze_scalar_expression(
                context(),
                expression,
                {},
                {});
        EXPECT_FALSE(r) << diagnostics();
        EXPECT_NE(count_error(), 0);
    }

    void invalid(sql_analyzer_code code, ast::scalar::expression const& expression) {
        invalid(expression);
        EXPECT_TRUE(find_error(code)) << diagnostics();
    }

    std::vector<::takatori::descriptor::variable> collect_let_variables(tscalar::expression const& expr) {
        if (expr.kind() == tscalar::unary::tag) {
            auto&& e = downcast<tscalar::unary>(expr);
            if (e.operator_kind() == tscalar::unary_operator::conditional_not) {
                return collect_let_variables(e.operand());
            }
        }
        if (expr.kind() != tscalar::let::tag) {
            return {};
        }
        auto&& let = downcast<tscalar::let>(expr);
        std::vector<::takatori::descriptor::variable> variables {};
        variables.reserve(let.variables().size());
        for (auto&& v : let.variables()) {
            variables.push_back(v.variable());
        }
        return variables;
    }
};

TEST_F(analyze_scalar_expression_case_test, case_searched) {
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::case_expression {
                    {
                            {
                                    ast::scalar::comparison_predicate {
                                            literal(number("1")),
                                            ast::scalar::comparison_operator::equals,
                                            literal(number("0")),
                                    },
                                    literal(number("100")),
                            },
                    },
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (tscalar::conditional {
            {
                    tscalar::conditional::alternative {
                            tscalar::compare {
                                    tscalar::comparison_operator::equal,
                                    immediate(1),
                                    immediate(0),
                            },
                            immediate(100),
                    },
            }
    }));
}

TEST_F(analyze_scalar_expression_case_test, case_searched_default) {
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::case_expression {
                    {
                            {
                                    ast::scalar::comparison_predicate {
                                            literal(number("1")),
                                            ast::scalar::comparison_operator::equals,
                                            literal(number("0")),
                                    },
                                    literal(number("100")),
                            },
                    },
                    literal(number("0")),
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (tscalar::conditional {
            {
                    tscalar::conditional::alternative {
                            tscalar::compare {
                                    tscalar::comparison_operator::equal,
                                    immediate(1),
                                    immediate(0),
                            },
                            immediate(100),
                    },
            },
            immediate(0),
    }));
}

TEST_F(analyze_scalar_expression_case_test, case_searched_multiple) {
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::case_expression {
                    {
                            {
                                    ast::scalar::comparison_predicate {
                                            literal(number("1")),
                                            ast::scalar::comparison_operator::equals,
                                            literal(number("0")),
                                    },
                                    literal(number("100")),
                            },
                            {
                                    ast::scalar::comparison_predicate {
                                            literal(number("2")),
                                            ast::scalar::comparison_operator::equals,
                                            literal(number("0")),
                                    },
                                    literal(number("200")),
                            },
                            {
                                    ast::scalar::comparison_predicate {
                                            literal(number("3")),
                                            ast::scalar::comparison_operator::equals,
                                            literal(number("0")),
                                    },
                                    literal(number("300")),
                            },
                    },
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (tscalar::conditional {
            {
                    tscalar::conditional::alternative {
                            tscalar::compare {
                                    tscalar::comparison_operator::equal,
                                    immediate(1),
                                    immediate(0),
                            },
                            immediate(100),
                    },
                    tscalar::conditional::alternative {
                            tscalar::compare {
                                    tscalar::comparison_operator::equal,
                                    immediate(2),
                                    immediate(0),
                            },
                            immediate(200),
                    },
                    tscalar::conditional::alternative {
                            tscalar::compare {
                                    tscalar::comparison_operator::equal,
                                    immediate(3),
                                    immediate(0),
                            },
                            immediate(300),
                    },
            }
    }));
}

TEST_F(analyze_scalar_expression_case_test, case_searched_empty) {
    invalid(sql_analyzer_code::malformed_syntax, ast::scalar::case_expression {
            std::initializer_list<ast::scalar::case_when_clause> {},
    });
}

TEST_F(analyze_scalar_expression_case_test, case_simple) {
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::case_expression {
                    literal(number("0")),
                    {
                            {
                                    literal(number("1")),
                                    literal(number("100")),
                            },
                    },
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    auto vars = collect_let_variables(*r);
    ASSERT_EQ(vars.size(), 1);

    EXPECT_EQ(*r, (tscalar::let {
            tscalar::let::variable { vars[0], immediate(0) },
            tscalar::conditional {
                    {
                            tscalar::conditional::alternative {
                                    tscalar::compare {
                                            tscalar::comparison_operator::equal,
                                            vref(vars[0]),
                                            immediate(1),
                                    },
                                    immediate(100),
                            },
                    }
            },
    }));
}

TEST_F(analyze_scalar_expression_case_test, case_simple_default) {
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::case_expression {
                    literal(number("0")),
                    {
                            {
                                    literal(number("1")),
                                    literal(number("100")),
                            },
                    },
                    literal(number("9999")),
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    auto vars = collect_let_variables(*r);
    ASSERT_EQ(vars.size(), 1);

    EXPECT_EQ(*r, (tscalar::let {
            tscalar::let::variable { vars[0], immediate(0) },
            tscalar::conditional {
                    {
                            tscalar::conditional::alternative {
                                    tscalar::compare {
                                            tscalar::comparison_operator::equal,
                                            vref(vars[0]),
                                            immediate(1),
                                    },
                                    immediate(100),
                            },
                    },
                    immediate(9999),
            },
    }));
}

TEST_F(analyze_scalar_expression_case_test, case_simple_multiple) {
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::case_expression {
                    literal(number("0")),
                    {
                            {
                                    literal(number("1")),
                                    literal(number("100")),
                            },
                            {
                                    literal(number("2")),
                                    literal(number("200")),
                            },
                            {
                                    literal(number("3")),
                                    literal(number("300")),
                            },
                    },
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    auto vars = collect_let_variables(*r);
    ASSERT_EQ(vars.size(), 1);

    EXPECT_EQ(*r, (tscalar::let {
            tscalar::let::variable { vars[0], immediate(0) },
            tscalar::conditional {
                    {
                            tscalar::conditional::alternative {
                                    tscalar::compare {
                                            tscalar::comparison_operator::equal,
                                            vref(vars[0]),
                                            immediate(1),
                                    },
                                    immediate(100),
                            },
                            tscalar::conditional::alternative {
                                    tscalar::compare {
                                            tscalar::comparison_operator::equal,
                                            vref(vars[0]),
                                            immediate(2),
                                    },
                                    immediate(200),
                            },
                            tscalar::conditional::alternative {
                                    tscalar::compare {
                                            tscalar::comparison_operator::equal,
                                            vref(vars[0]),
                                            immediate(3),
                                    },
                                    immediate(300),
                            },
                    }
            },
    }));
}

TEST_F(analyze_scalar_expression_case_test, case_simple_empty) {
    invalid(sql_analyzer_code::malformed_syntax, ast::scalar::case_expression {
            literal(number("1")),
            std::initializer_list<ast::scalar::case_when_clause> {},
    });
}

TEST_F(analyze_scalar_expression_case_test, nullif) {
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::builtin_function_invocation {
                    ast::scalar::builtin_function_kind::nullif,
                    {
                            literal(number("1")),
                            literal(number("2")),
                    },
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    auto vars = collect_let_variables(*r);
    ASSERT_EQ(vars.size(), 1);

    EXPECT_EQ(*r, (tscalar::let {
            tscalar::let::variable { vars[0], immediate(1) },
            tscalar::conditional {
                    {
                            tscalar::conditional::alternative {
                                    tscalar::compare {
                                            tscalar::comparison_operator::equal,
                                            vref(vars[0]),
                                            immediate(2),
                                    },
                                    tscalar::immediate {
                                            tvalue::unknown {},
                                            ttype::int8 {},
                                    },
                            },
                    },
                    vref(vars[0]),
            },
    }));
}

TEST_F(analyze_scalar_expression_case_test, nullif_invalid) {
    invalid(sql_analyzer_code::malformed_syntax, ast::scalar::builtin_function_invocation {
            ast::scalar::builtin_function_kind::nullif,
            {
                    literal(number("1")),
            },
    });
}

} // namespace mizugaki::analyzer::details
