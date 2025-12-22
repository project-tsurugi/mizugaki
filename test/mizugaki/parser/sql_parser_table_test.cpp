#include <mizugaki/parser/sql_parser.h>

#include <gtest/gtest.h>

#include <mizugaki/ast/statement/select_statement.h>

#include <mizugaki/ast/query/query.h>
#include <mizugaki/ast/query/select_asterisk.h>
#include <mizugaki/ast/query/table_reference.h>

#include <mizugaki/ast/table/table_reference.h>
#include <mizugaki/ast/table/unnest.h>
#include <mizugaki/ast/table/join.h>
#include <mizugaki/ast/table/subquery.h>
#include <mizugaki/ast/table/apply.h>
#include <mizugaki/ast/table/join_condition.h>
#include <mizugaki/ast/table/join_columns.h>

#include <mizugaki/ast/scalar/literal_expression.h>
#include <mizugaki/ast/scalar/variable_reference.h>
#include <mizugaki/ast/scalar/comparison_predicate.h>
#include <mizugaki/ast/scalar/value_constructor.h>

#include <mizugaki/ast/name/simple.h>

#include "utils.h"

namespace mizugaki::parser {

using namespace testing;

class sql_parser_table_test : public ::testing::Test {
protected:
    sql_parser_result parse(std::string_view str, sql_parser parser = default_parser()) {
        using ::takatori::util::string_builder;
        return parser("-", string_builder {}
                << "SELECT * FROM " << str << ";"
                << string_builder::to_string);
    }
};

static table::expression const& extract(sql_parser::result_type const& result) {
    auto&& stmt = result.value()->statements().at(0);
    auto&& select = downcast<statement::select_statement>(*stmt);
    auto&& q = downcast<query::query>(*select.expression());
    return *q.from()[0];
}

TEST_F(sql_parser_table_test, table_reference) {
    auto result = parse("T0");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (table::table_reference {
            name::simple { "T0" },
    }));
}

TEST_F(sql_parser_table_test, table_reference_correlation) {
    auto result = parse("T0 AS X");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (table::table_reference {
            name::simple { "T0" },
            {
                name::simple { "X" },
            }
    }));
}

TEST_F(sql_parser_table_test, table_reference_correlation_columns) {
    auto result = parse("T0 AS X (C0, C1, C2)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (table::table_reference {
            name::simple { "T0" },
            {
                    {
                            name::simple { "X" },
                            {
                                    name::simple { "C0" },
                                    name::simple { "C1" },
                                    name::simple { "C2" },
                            },
                    }
            },
    }));
}

TEST_F(sql_parser_table_test, derived_table) {
    auto result = parse("(TABLE T0) AS X");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (table::subquery {
            query::table_reference {
                    name::simple { "T0" },
            },
            {
                    name::simple { "X" },
            }
    }));
}

TEST_F(sql_parser_table_test, lateral_derived_table) {
    auto result = parse("LATERAL (TABLE T0) AS X");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (table::subquery {
            query::table_reference {
                    name::simple { "T0" },
            },
            {
                    name::simple { "X" },
            },
            true,
    }));
}

TEST_F(sql_parser_table_test, collection_derived_table) {
    auto result = parse("UNNEST (a) AS X");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (table::unnest {
            scalar::variable_reference {
                    name::simple { "a" },
            },
            {
                    name::simple { "X" },
            },
    }));
}

TEST_F(sql_parser_table_test, collection_derived_table_with_ordinaliry) {
    auto result = parse("UNNEST (a) WITH ORDINALITY AS X");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (table::unnest {
            scalar::variable_reference {
                    name::simple { "a" },
            },
            {
                    name::simple { "X" },
            },
            true,
    }));
}

TEST_F(sql_parser_table_test, only_spec) {
    auto result = parse("ONLY (T0)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (table::table_reference {
            name::simple { "T0" },
            {},
            true,
    }));
}

TEST_F(sql_parser_table_test, only_spec_correlation) {
    auto result = parse("ONLY (T0) AS X");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (table::table_reference {
            name::simple { "T0" },
            {
                    name::simple { "X" },
            },
            true,
    }));
}

TEST_F(sql_parser_table_test, cross_join) {
    auto result = parse("T0 CROSS JOIN T1");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (table::join {
            table::table_reference {
                    name::simple { "T0" },
            },
            table::join_type::cross,
            table::table_reference {
                    name::simple { "T1" },
            },
    }));
}

TEST_F(sql_parser_table_test, join_on) {
    auto result = parse("T0 JOIN T1 ON A = B");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (table::join {
            table::table_reference {
                    name::simple { "T0" },
            },
            table::join_type::inner,
            table::table_reference {
                    name::simple { "T1" },
            },
            table::join_condition {
                    scalar::comparison_predicate {
                            scalar::variable_reference {
                                    name::simple { "A" },
                            },
                            scalar::comparison_operator::equals,
                            scalar::variable_reference {
                                    name::simple { "B" },
                            },
                    },
            },
    }));
}

TEST_F(sql_parser_table_test, join_using) {
    auto result = parse("T0 JOIN T1 USING (A, B)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (table::join {
            table::table_reference {
                    name::simple { "T0" },
            },
            table::join_type::inner,
            table::table_reference {
                    name::simple { "T1" },
            },
            table::join_columns {
                    name::simple { "A" },
                    name::simple { "B" },
            },
    }));
}

TEST_F(sql_parser_table_test, left_outer_join) {
    auto result = parse("T0 LEFT OUTER JOIN T1 ON A = B");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (table::join {
            table::table_reference {
                    name::simple { "T0" },
            },
            table::join_type::left_outer,
            table::table_reference {
                    name::simple { "T1" },
            },
            table::join_condition {
                    scalar::comparison_predicate {
                            scalar::variable_reference {
                                    name::simple { "A" },
                            },
                            scalar::comparison_operator::equals,
                            scalar::variable_reference {
                                    name::simple { "B" },
                            },
                    },
            },
    }));
}

TEST_F(sql_parser_table_test, right_outer_join) {
    auto result = parse("T0 RIGHT JOIN T1 ON A = B");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (table::join {
            table::table_reference {
                    name::simple { "T0" },
            },
            table::join_type::right_outer,
            table::table_reference {
                    name::simple { "T1" },
            },
            table::join_condition {
                    scalar::comparison_predicate {
                            scalar::variable_reference {
                                    name::simple { "A" },
                            },
                            scalar::comparison_operator::equals,
                            scalar::variable_reference {
                                    name::simple { "B" },
                            },
                    },
            },
    }));
}

TEST_F(sql_parser_table_test, full_outer_join) {
    auto result = parse("T0 FULL OUTER JOIN T1 ON A = B");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (table::join {
            table::table_reference {
                    name::simple { "T0" },
            },
            table::join_type::full_outer,
            table::table_reference {
                    name::simple { "T1" },
            },
            table::join_condition {
                    scalar::comparison_predicate {
                            scalar::variable_reference {
                                    name::simple { "A" },
                            },
                            scalar::comparison_operator::equals,
                            scalar::variable_reference {
                                    name::simple { "B" },
                            },
                    },
            },
    }));
}

TEST_F(sql_parser_table_test, natural_join) {
    auto result = parse("T0 NATURAL JOIN T1");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (table::join {
            table::table_reference {
                    name::simple { "T0" },
            },
            table::join_type::natural_inner,
            table::table_reference {
                    name::simple { "T1" },
            },
    }));
}

TEST_F(sql_parser_table_test, union_join) {
    auto result = parse("T0 UNION JOIN T1");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (table::join {
            table::table_reference {
                    name::simple { "T0" },
            },
            table::join_type::union_,
            table::table_reference {
                    name::simple { "T1" },
            },
    }));
}

TEST_F(sql_parser_table_test, apply) {
        auto result = parse("T0 APPLY f(A) x");
        ASSERT_TRUE(result) << diagnostics(result);

        EXPECT_EQ(extract(result), (table::apply {
                table::table_reference {
                        name::simple { "T0" },
                },
                name::simple { "f" },
                {
                        scalar::variable_reference {
                                name::simple { "A" },
                        },
                },
                {
                        name::simple { "x" },
                },
        }));
}

TEST_F(sql_parser_table_test, apply_arguments) {
        auto result = parse("T0 APPLY f(A, B, C) x");
        ASSERT_TRUE(result) << diagnostics(result);

        EXPECT_EQ(extract(result), (table::apply {
                table::table_reference {
                        name::simple { "T0" },
                },
                name::simple { "f" },
                {
                        scalar::variable_reference {
                                name::simple { "A" },
                        },
                        scalar::variable_reference {
                                name::simple { "B" },
                        },
                        scalar::variable_reference {
                                name::simple { "C" },
                        },
                },
                {
                        name::simple { "x" },
                },
        }));
}

TEST_F(sql_parser_table_test, apply_cross) {
        auto result = parse("T0 CROSS APPLY f(A) x");
        ASSERT_TRUE(result) << diagnostics(result);

        EXPECT_EQ(extract(result), (table::apply {
                table::table_reference {
                        name::simple { "T0" },
                },
                name::simple { "f" },
                {
                        scalar::variable_reference {
                                name::simple { "A" },
                        },
                },
                {
                        name::simple { "x" },
                },
                {
                        ast::table::apply_type::cross,
                }
        }));
}

TEST_F(sql_parser_table_test, apply_outer) {
        auto result = parse("T0 OUTER APPLY f(A) x");
        ASSERT_TRUE(result) << diagnostics(result);

        EXPECT_EQ(extract(result), (table::apply {
                table::table_reference {
                        name::simple { "T0" },
                },
                name::simple { "f" },
                {
                        scalar::variable_reference {
                                name::simple { "A" },
                        },
                },
                {
                        name::simple { "x" },
                },
                {
                        ast::table::apply_type::outer,
                }
        }));
}

TEST_F(sql_parser_table_test, apply_correlation_as) {
        auto result = parse("T0 CROSS APPLY f(A) AS x");
        ASSERT_TRUE(result) << diagnostics(result);

        EXPECT_EQ(extract(result), (table::apply {
                table::table_reference {
                        name::simple { "T0" },
                },
                name::simple { "f" },
                {
                        scalar::variable_reference {
                                name::simple { "A" },
                        },
                },
                {
                        name::simple { "x" },
                },
                {
                        ast::table::apply_type::cross,
                }
        }));
}

TEST_F(sql_parser_table_test, apply_correlation_columns) {
        auto result = parse("T0 APPLY f(A) x(a, b, c)");
        ASSERT_TRUE(result) << diagnostics(result);

        EXPECT_EQ(extract(result), (table::apply {
                table::table_reference {
                        name::simple { "T0" },
                },
                name::simple { "f" },
                {
                        scalar::variable_reference {
                                name::simple { "A" },
                        },
                },
                {
                        name::simple { "x" },
                        {
                                name::simple { "a" },
                                name::simple { "b" },
                                name::simple { "c" },
                        }
                },
        }));
}

} // namespace mizugaki::parser
