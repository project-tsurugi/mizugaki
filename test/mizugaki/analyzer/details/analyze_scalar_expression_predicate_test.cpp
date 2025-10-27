#include <mizugaki/analyzer/details/analyze_scalar_expression.h>

#include <gtest/gtest.h>

#include <takatori/type/primitive.h>

#include <takatori/scalar/binary.h>
#include <takatori/scalar/compare.h>
#include <takatori/scalar/let.h>
#include <takatori/scalar/match.h>
#include <takatori/scalar/unary.h>

#include <mizugaki/ast/scalar/between_predicate.h>
#include <mizugaki/ast/scalar/binary_expression.h>
#include <mizugaki/ast/scalar/comparison_predicate.h>
#include <mizugaki/ast/scalar/in_predicate.h>
#include <mizugaki/ast/scalar/pattern_match_predicate.h>

#include <mizugaki/ast/query/table_value_constructor.h>
#include <mizugaki/ast/query/table_reference.h>

#include "test_parent.h"

namespace mizugaki::analyzer::details {

using namespace ::mizugaki::analyzer::testing;

class analyze_scalar_expression_predicate_test : public test_parent {
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
        EXPECT_TRUE(find_error(code));
    }

    ast::scalar::literal_expression erroneous_expression() {
        return literal(string("INVALID"));
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

TEST_F(analyze_scalar_expression_predicate_test, comparison_predicate_equals) {
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
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (tscalar::compare {
            to,
            immediate(1),
            immediate(2),
    }));
}

TEST_F(analyze_scalar_expression_predicate_test, comparison_predicate_not_equals) {
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
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (tscalar::compare {
            to,
            immediate(1),
            immediate(2),
    }));
}

TEST_F(analyze_scalar_expression_predicate_test, comparison_predicate_not_equals_alternative) {
    auto from = ast::scalar::comparison_operator::not_equals_alternative;
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
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (tscalar::compare {
            to,
            immediate(1),
            immediate(2),
    }));
}

TEST_F(analyze_scalar_expression_predicate_test, comparison_predicate_less_than) {
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
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (tscalar::compare {
            to,
            immediate(1),
            immediate(2),
    }));
}

TEST_F(analyze_scalar_expression_predicate_test, comparison_predicate_greater_than) {
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
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (tscalar::compare {
            to,
            immediate(1),
            immediate(2),
    }));
}

TEST_F(analyze_scalar_expression_predicate_test, comparison_predicate_less_than_or_equals) {
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
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (tscalar::compare {
            to,
            immediate(1),
            immediate(2),
    }));
}

TEST_F(analyze_scalar_expression_predicate_test, comparison_predicate_greater_than_or_equals) {
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
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (tscalar::compare {
            to,
            immediate(1),
            immediate(2),
    }));
}

TEST_F(analyze_scalar_expression_predicate_test, comparison_predicate_invalid_left) {
    invalid(ast::scalar::comparison_predicate {
            erroneous_expression(),
            ast::scalar::comparison_operator::equals,
            literal(string("'right'")),
    });
}

TEST_F(analyze_scalar_expression_predicate_test, comparison_predicate_invalid_right) {
    invalid(ast::scalar::comparison_predicate {
            literal(string("'left'")),
            ast::scalar::comparison_operator::equals,
            erroneous_expression(),
    });
}

TEST_F(analyze_scalar_expression_predicate_test, between_predicate) {
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::between_predicate {
                    literal(number("1")),
                    literal(number("2")),
                    literal(number("3")),
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    auto vs = collect_let_variables(*r);
    ASSERT_EQ(vs.size(), 1);
    EXPECT_EQ(*r, (tscalar::let {
        tscalar::let::variable { vs[0], immediate(1) },
        tscalar::binary {
                tscalar::binary_operator::conditional_and,
                tscalar::compare {
                        tscalar::comparison_operator::less_equal,
                        immediate(2),
                        vref(vs[0]),
                },
                tscalar::compare {
                        tscalar::comparison_operator::less_equal,
                        vref(vs[0]),
                        immediate(3),
                },
        },
    }));
}

TEST_F(analyze_scalar_expression_predicate_test, between_predicate_not) {
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::between_predicate {
                    literal(number("1")),
                    literal(number("2")),
                    literal(number("3")),
                    true,
                    ast::scalar::between_operator::asymmetric,
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    auto vs = collect_let_variables(*r);
    ASSERT_EQ(vs.size(), 1);
    EXPECT_EQ(*r, (tscalar::unary {
            tscalar::unary_operator::conditional_not,
            tscalar::let {
                tscalar::let::variable { vs[0], immediate(1) },
                        tscalar::binary {
                                tscalar::binary_operator::conditional_and,
                                tscalar::compare {
                                        tscalar::comparison_operator::less_equal,
                                        immediate(2),
                                        vref(vs[0]),
                                },
                                tscalar::compare {
                                        tscalar::comparison_operator::less_equal,
                                        vref(vs[0]),
                                        immediate(3),
                                },
                        },
            },
    }));
}

TEST_F(analyze_scalar_expression_predicate_test, between_predicate_symmetric) {
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::between_predicate {
                    literal(number("1")),
                    literal(number("2")),
                    literal(number("3")),
                    false,
                    ast::scalar::between_operator::symmetric,
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    auto vs = collect_let_variables(*r);
    ASSERT_EQ(vs.size(), 3);
    EXPECT_EQ(*r, (tscalar::let {
            {
                    tscalar::let::variable { vs[0], immediate(1) },
                    tscalar::let::variable { vs[1], immediate(2) },
                    tscalar::let::variable { vs[2], immediate(3) },
            },
            tscalar::binary {
                    tscalar::binary_operator::conditional_or,
                    tscalar::binary {
                            tscalar::binary_operator::conditional_and,
                            tscalar::compare {
                                    tscalar::comparison_operator::less_equal,
                                    vref(vs[1]),
                                    vref(vs[0]),
                            },
                            tscalar::compare {
                                    tscalar::comparison_operator::less_equal,
                                    vref(vs[0]),
                                    vref(vs[2]),
                            },
                    },
                    tscalar::binary {
                            tscalar::binary_operator::conditional_and,
                            tscalar::compare {
                                    tscalar::comparison_operator::less_equal,
                                    vref(vs[2]),
                                    vref(vs[0]),
                            },
                            tscalar::compare {
                                    tscalar::comparison_operator::less_equal,
                                    vref(vs[0]),
                                    vref(vs[1]),
                            },
                    },
            },
    }));
}

TEST_F(analyze_scalar_expression_predicate_test, in_predicate_values) {
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::in_predicate {
                    literal(number("1")),
                    ast::query::table_value_constructor {
                            literal(number("2")),
                    }
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    auto vs = collect_let_variables(*r);
    ASSERT_EQ(vs.size(), 1);
    EXPECT_EQ(*r, (tscalar::let {
            tscalar::let::variable { vs[0], immediate(1) },
            tscalar::compare {
                    tscalar::comparison_operator::equal,
                    vref(vs[0]),
                    immediate(2),
            },
    }));
}

TEST_F(analyze_scalar_expression_predicate_test, in_predicate_values_multiple) {
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::in_predicate {
                    literal(number("1")),
                    ast::query::table_value_constructor {
                            literal(number("2")),
                            literal(number("3")),
                            literal(number("4")),
                    }
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    auto vs = collect_let_variables(*r);
    ASSERT_EQ(vs.size(), 1);
    EXPECT_EQ(*r, (tscalar::let {
            tscalar::let::variable { vs[0], immediate(1) },
            tscalar::binary {
                    tscalar::binary_operator::conditional_or,
                    tscalar::binary {
                            tscalar::binary_operator::conditional_or,
                            tscalar::compare {
                                    tscalar::comparison_operator::equal,
                                    vref(vs[0]),
                                    immediate(2),
                            },
                            tscalar::compare {
                                    tscalar::comparison_operator::equal,
                                    vref(vs[0]),
                                    immediate(3),
                            },
                    },
                    tscalar::compare {
                            tscalar::comparison_operator::equal,
                            vref(vs[0]),
                            immediate(4),
                    },
            },
    }));
}

TEST_F(analyze_scalar_expression_predicate_test, in_predicate_values_empty) {
    invalid(sql_analyzer_code::malformed_syntax, ast::scalar::in_predicate {
            literal(number("1")),
            ast::query::table_value_constructor {}
    });
}

TEST_F(analyze_scalar_expression_predicate_test, in_predicate_values_not) {
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::in_predicate {
                    literal(number("1")),
                    ast::query::table_value_constructor {
                            literal(number("2")),
                            literal(number("3")),
                            literal(number("4")),
                    },
                    true,
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    auto vs = collect_let_variables(*r);
    ASSERT_EQ(vs.size(), 1);
    EXPECT_EQ(*r, (tscalar::unary {
            tscalar::unary_operator::conditional_not,
            tscalar::let {
                    tscalar::let::variable { vs[0], immediate(1) },
                    tscalar::binary {
                            tscalar::binary_operator::conditional_or,
                            tscalar::binary {
                                    tscalar::binary_operator::conditional_or,
                                    tscalar::compare {
                                            tscalar::comparison_operator::equal,
                                            vref(vs[0]),
                                            immediate(2),
                                    },
                                    tscalar::compare {
                                            tscalar::comparison_operator::equal,
                                            vref(vs[0]),
                                            immediate(3),
                                    },
                            },
                            tscalar::compare {
                                    tscalar::comparison_operator::equal,
                                    vref(vs[0]),
                                    immediate(4),
                            },
                    },
            }
    }));
}

TEST_F(analyze_scalar_expression_predicate_test, in_predicate_query) {
    install_table("t");
    invalid(sql_analyzer_code::unsupported_feature, ast::scalar::in_predicate {
            literal(number("1")),
            ast::query::table_reference {
                    id("t"),
            },
    });
}

TEST_F(analyze_scalar_expression_predicate_test, pattern_match_predicate_simple) {
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::pattern_match_predicate {
                    literal(string("'input'")),
                    ast::scalar::pattern_match_operator::like,
                    literal(string("'p%'")),
                    {},
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (tscalar::match {
            tscalar::match_operator::like,
            immediate("input"),
            immediate("p%"),
            immediate(""),
    }));
}

TEST_F(analyze_scalar_expression_predicate_test, pattern_match_predicate_similar_to) {
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::pattern_match_predicate {
                    literal(string("'input'")),
                    ast::scalar::pattern_match_operator::similar_to,
                    literal(string("'p.*'")),
                    {},
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (tscalar::match {
            tscalar::match_operator::similar,
            immediate("input"),
            immediate("p.*"),
            immediate(""),
    }));
}

TEST_F(analyze_scalar_expression_predicate_test, pattern_match_predicate_escape) {
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::pattern_match_predicate {
                    literal(string("'input'")),
                    ast::scalar::pattern_match_operator::like,
                    literal(string("'p\\%'")),
                    literal(string("'\\'")),
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (tscalar::match {
            tscalar::match_operator::like,
            immediate("input"),
            immediate("p\\%"),
            immediate("\\"),
    }));
}

TEST_F(analyze_scalar_expression_predicate_test, pattern_match_predicate_not) {
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::pattern_match_predicate {
                    literal(string("'input'")),
                    ast::scalar::pattern_match_operator::like,
                    literal(string("'p%'")),
                    {},
                    { true },
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (tscalar::unary {
            tscalar::unary_operator::conditional_not,
            tscalar::match {
                    tscalar::match_operator::like,
                    immediate("input"),
                    immediate("p%"),
                    immediate(""),
            }
    }));
}

TEST_F(analyze_scalar_expression_predicate_test, pattern_match_predicate_invalid_input) {
    invalid(ast::scalar::pattern_match_predicate {
            erroneous_expression(),
            ast::scalar::pattern_match_operator::like,
            literal(string("'p%'")),
            {},
    });
}

TEST_F(analyze_scalar_expression_predicate_test, pattern_match_predicate_invalid_pattern) {
    invalid(ast::scalar::pattern_match_predicate {
            literal(string("'input'")),
            ast::scalar::pattern_match_operator::like,
            erroneous_expression(),
            {},
    });
}

TEST_F(analyze_scalar_expression_predicate_test, pattern_match_predicate_invalid_escape) {
    invalid(ast::scalar::pattern_match_predicate {
            literal(string("'input'")),
            ast::scalar::pattern_match_operator::like,
            literal(string("'p%'")),
            erroneous_expression(),
    });
}

} // namespace mizugaki::analyzer::details
