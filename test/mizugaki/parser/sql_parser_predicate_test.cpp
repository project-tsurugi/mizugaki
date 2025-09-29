#include <mizugaki/parser/sql_parser.h>

#include <gtest/gtest.h>

#include <mizugaki/ast/statement/select_statement.h>

#include <mizugaki/ast/query/query.h>
#include <mizugaki/ast/query/table_reference.h>
#include <mizugaki/ast/query/table_value_constructor.h>

#include <mizugaki/ast/scalar/comparison_predicate.h>
#include <mizugaki/ast/scalar/quantified_comparison_predicate.h>
#include <mizugaki/ast/scalar/between_predicate.h>
#include <mizugaki/ast/scalar/in_predicate.h>
#include <mizugaki/ast/scalar/pattern_match_predicate.h>
#include <mizugaki/ast/scalar/table_predicate.h>

#include <mizugaki/ast/scalar/literal_expression.h>
#include <mizugaki/ast/scalar/variable_reference.h>
#include <mizugaki/ast/scalar/binary_expression.h>
#include <mizugaki/ast/scalar/unary_expression.h>
#include <mizugaki/ast/scalar/value_constructor.h>

#include <mizugaki/ast/literal/boolean.h>
#include <mizugaki/ast/literal/special.h>

#include "utils.h"

namespace mizugaki::parser {

using namespace testing;

class sql_parser_predicate_test : public ::testing::Test {
protected:
    sql_parser_result parse(std::string_view str, sql_parser parser = default_parser()) {
        using ::takatori::util::string_builder;
        return parser("-", string_builder {}
                << "SELECT * FROM T0 WHERE " << str << ";"
                << string_builder::to_string);
    }
};

static scalar::expression const& extract(sql_parser::result_type const& result) {
    auto&& stmt = result.value()->statements().at(0);
    auto&& select = downcast<statement::select_statement>(*stmt);
    auto&& q = downcast<query::query>(*select.expression());
    return *q.where();
}

TEST_F(sql_parser_predicate_test, or) {
    auto result = parse("a OR b");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::binary_expression {
            v("a"),
            scalar::binary_operator::or_,
            v("b"),
    }));
}

TEST_F(sql_parser_predicate_test, and) {
    auto result = parse("a AND b");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::binary_expression {
            v("a"),
            scalar::binary_operator::and_,
            v("b"),
    }));
}

TEST_F(sql_parser_predicate_test, not) {
    auto result = parse("NOT a");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::unary_expression {
            scalar::unary_operator::not_,
            v("a"),
    }));
}

TEST_F(sql_parser_predicate_test, conditional_operator_precedence) {
    auto result = parse("v0 AND v1 OR NOT v2 AND NOT v3");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::binary_expression {
            scalar::binary_expression {
                    v("v0"),
                    scalar::binary_operator::and_,
                    v("v1"),
            },
            scalar::binary_operator::or_,
            scalar::binary_expression {
                    scalar::unary_expression {
                            scalar::unary_operator::not_,
                            v("v2"),
                    },
                    scalar::binary_operator::and_,
                    scalar::unary_expression {
                            scalar::unary_operator::not_,
                            v("v3"),
                    },
            },
    }));
}

TEST_F(sql_parser_predicate_test, is_true) {
    auto result = parse("a IS TRUE");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::binary_expression {
            v("a"),
            scalar::binary_operator::is,
            scalar::literal_expression {
                    literal::boolean {
                            literal::boolean_kind::true_,
                    }
            },
    }));
}

TEST_F(sql_parser_predicate_test, is_not_true) {
    auto result = parse("a IS NOT TRUE");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::binary_expression {
            v("a"),
            scalar::binary_operator::is_not,
            scalar::literal_expression {
                    literal::boolean {
                            literal::boolean_kind::true_,
                    }
            },
    }));
}

TEST_F(sql_parser_predicate_test, is_false) {
    auto result = parse("a IS FALSE");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::binary_expression {
            v("a"),
            scalar::binary_operator::is,
            scalar::literal_expression {
                    literal::boolean {
                            literal::boolean_kind::false_,
                    }
            },
    }));
}

TEST_F(sql_parser_predicate_test, is_unknown) {
    auto result = parse("a IS UNKNOWN");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::binary_expression {
            v("a"),
            scalar::binary_operator::is,
            scalar::literal_expression {
                    literal::boolean {
                            literal::boolean_kind::unknown,
                    }
            },
    }));
}

TEST_F(sql_parser_predicate_test, is_null) {
    auto result = parse("a IS NULL");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::binary_expression {
            v("a"),
            scalar::binary_operator::is,
            scalar::literal_expression {
                    literal::null {},
            },
    }));
}

TEST_F(sql_parser_predicate_test, comparison_predicate_eq) {
    auto result = parse("a = b");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::comparison_predicate {
            v("a"),
            scalar::comparison_operator::equals,
            v("b"),
        }));
}

TEST_F(sql_parser_predicate_test, comparison_predicate_ne) {
    auto result = parse("a <> b");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::comparison_predicate {
            v("a"),
            scalar::comparison_operator::not_equals,
            v("b"),
    }));
}

TEST_F(sql_parser_predicate_test, comparison_predicate_ne_alternative) {
    auto result = parse("a != b");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::comparison_predicate {
            v("a"),
            scalar::comparison_operator::not_equals_alternative,
            v("b"),
    }));
}

TEST_F(sql_parser_predicate_test, comparison_predicate_lt) {
    auto result = parse("a < b");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::comparison_predicate {
            v("a"),
            scalar::comparison_operator::less_than,
            v("b"),
    }));
}

TEST_F(sql_parser_predicate_test, comparison_predicate_gt) {
    auto result = parse("a > b");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::comparison_predicate {
            v("a"),
            scalar::comparison_operator::greater_than,
            v("b"),
    }));
}

TEST_F(sql_parser_predicate_test, comparison_predicate_le) {
    auto result = parse("a <= b");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::comparison_predicate {
            v("a"),
            scalar::comparison_operator::less_than_or_equals,
            v("b"),
    }));
}

TEST_F(sql_parser_predicate_test, comparison_predicate_ge) {
    auto result = parse("a >= b");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::comparison_predicate {
            v("a"),
            scalar::comparison_operator::greater_than_or_equals,
            v("b"),
    }));
}

TEST_F(sql_parser_predicate_test, between_predicate) {
    auto result = parse("a BETWEEN 1 AND 2");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::between_predicate {
            v("a"),
            int_literal("1"),
            int_literal("2"),
    }));
}

TEST_F(sql_parser_predicate_test, between_predicate_not) {
    auto result = parse("a NOT BETWEEN 1 AND 2");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::between_predicate {
            v("a"),
            int_literal("1"),
            int_literal("2"),
            true,
    }));
}

TEST_F(sql_parser_predicate_test, between_predicate_symmetric) {
    auto result = parse("a BETWEEN SYMMETRIC 1 AND 2");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::between_predicate {
            v("a"),
            int_literal("1"),
            int_literal("2"),
            false,
            scalar::between_operator::symmetric,
    }));
}

TEST_F(sql_parser_predicate_test, between_predicate_asymmetric) {
    auto result = parse("a BETWEEN ASYMMETRIC 1 AND 2");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::between_predicate {
            v("a"),
            int_literal("1"),
            int_literal("2"),
            false,
            scalar::between_operator::asymmetric,
    }));
}

TEST_F(sql_parser_predicate_test, in_predicate_subquery) {
    auto result = parse("(1, 2) IN (TABLE a)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::in_predicate {
            scalar::value_constructor {
                    int_literal("1"),
                    int_literal("2"),
            },
            query::table_reference {
                    name::simple { "a" },
            },
    }));
}

TEST_F(sql_parser_predicate_test, in_predicate_values) {
    auto result = parse("1 IN (u, v, w)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::in_predicate {
            int_literal("1"),
            query::table_value_constructor {
                    v("u"),
                    v("v"),
                    v("w"),
            },
    }));
}

TEST_F(sql_parser_predicate_test, in_predicate_not) {
    auto result = parse("1 NOT IN (TABLE a)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::in_predicate {
            int_literal("1"),
            query::table_reference {
                    name::simple { "a" },
            },
            true,
    }));
}

TEST_F(sql_parser_predicate_test, like_predicate) {
    auto result = parse("a LIKE b");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::pattern_match_predicate {
            v("a"),
            scalar::pattern_match_operator::like,
            v("b"),
    }));
}

TEST_F(sql_parser_predicate_test, like_predicate_escape) {
    auto result = parse("a LIKE b ESCAPE e");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::pattern_match_predicate {
            v("a"),
            scalar::pattern_match_operator::like,
            v("b"),
            v("e"),
    }));
}

TEST_F(sql_parser_predicate_test, like_predicate_not) {
    auto result = parse("a NOT LIKE b");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::pattern_match_predicate {
            v("a"),
            scalar::pattern_match_operator::like,
            v("b"),
            {},
            true,
    }));
}

TEST_F(sql_parser_predicate_test, similar_to_predicate) {
    auto result = parse("a SIMILAR TO b");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::pattern_match_predicate {
            v("a"),
            scalar::pattern_match_operator::similar_to,
            v("b"),
    }));
}

TEST_F(sql_parser_predicate_test, similar_to_predicate_escape) {
    auto result = parse("a SIMILAR TO b ESCAPE e");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::pattern_match_predicate {
            v("a"),
            scalar::pattern_match_operator::similar_to,
            v("b"),
            v("e"),
    }));
}

TEST_F(sql_parser_predicate_test, similar_to_predicate_not) {
    auto result = parse("a NOT SIMILAR TO b");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::pattern_match_predicate {
            v("a"),
            scalar::pattern_match_operator::similar_to,
            v("b"),
            {},
            true,
    }));
}

TEST_F(sql_parser_predicate_test, quantified_comparison_predicate_all) {
    auto result = parse("a = ALL (TABLE t)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::quantified_comparison_predicate {
            v("a"),
            scalar::comparison_operator::equals,
            scalar::quantifier::all,
            query::table_reference {
                    name::simple { "t" },
            },
    }));
}

TEST_F(sql_parser_predicate_test, quantified_comparison_predicate_any) {
    auto result = parse("a = ANY (TABLE t)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::quantified_comparison_predicate {
            v("a"),
            scalar::comparison_operator::equals,
            scalar::quantifier::any,
            query::table_reference {
                    name::simple { "t" },
            },
    }));
}

TEST_F(sql_parser_predicate_test, quantified_comparison_predicate_some) {
    auto result = parse("a = SOME (TABLE t)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::quantified_comparison_predicate {
            v("a"),
            scalar::comparison_operator::equals,
            scalar::quantifier::some,
            query::table_reference {
                    name::simple { "t" },
            },
    }));
}

TEST_F(sql_parser_predicate_test, exists_predicate) {
    auto result = parse("EXISTS (TABLE a)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::table_predicate {
            scalar::table_operator::exists,
            query::table_reference {
                    name::simple { "a" },
            },
    }));
}

TEST_F(sql_parser_predicate_test, exists_predicate_not) {
    auto result = parse("NOT EXISTS (TABLE a)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::unary_expression {
            scalar::unary_operator::not_,
            scalar::table_predicate {
                    scalar::table_operator::exists,
                    query::table_reference {
                            name::simple { "a" },
                    },
            },
    }));
}

TEST_F(sql_parser_predicate_test, unique_predicate) {
    auto result = parse("UNIQUE (TABLE a)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::table_predicate {
            scalar::table_operator::unique,
            query::table_reference {
                    name::simple { "a" },
            },
    }));
}

TEST_F(sql_parser_predicate_test, unique_predicate_not) {
    auto result = parse("NOT UNIQUE (TABLE a)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::unary_expression {
            scalar::unary_operator::not_,
            scalar::table_predicate {
                    scalar::table_operator::unique,
                    query::table_reference {
                            name::simple { "a" },
                    },
            },
    }));
}

TEST_F(sql_parser_predicate_test, overlaps_predicate) {
    auto result = parse("(u0, v0) OVERLAPS (u1, v1)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::binary_expression {
            scalar::value_constructor {
                    v("u0"),
                    v("v0"),
            },
            scalar::binary_operator::overlaps,
            scalar::value_constructor {
                    v("u1"),
                    v("v1"),
            },
    }));
}

TEST_F(sql_parser_predicate_test, extension_contains_operator) {
    auto result = parse("a <@ b");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::binary_expression {
            v("a"),
            scalar::binary_operator::contains,
            v("b"),
    }));
}

TEST_F(sql_parser_predicate_test, extension_is_contained_by_operator) {
    auto result = parse("a @> b");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (scalar::binary_expression {
            v("a"),
            scalar::binary_operator::is_contained_by,
            v("b"),
    }));
}

} // namespace mizugaki::parser
