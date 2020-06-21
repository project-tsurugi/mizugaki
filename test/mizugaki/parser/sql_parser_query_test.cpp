#include <mizugaki/parser/sql_parser.h>

#include <gtest/gtest.h>

#include <mizugaki/ast/statement/select_statement.h>

#include <mizugaki/ast/query/query.h>
#include <mizugaki/ast/query/select_asterisk.h>
#include <mizugaki/ast/query/select_column.h>
#include <mizugaki/ast/query/grouping_column.h>

#include <mizugaki/ast/table/table_reference.h>

#include <mizugaki/ast/scalar/literal_expression.h>
#include <mizugaki/ast/scalar/variable_reference.h>
#include <mizugaki/ast/scalar/comparison_predicate.h>

#include <mizugaki/ast/name/simple.h>

#include "utils.h"

namespace mizugaki::parser {

using namespace testing;

class sql_parser_query_test : public ::testing::Test {};

static query::expression const& extract(sql_parser::result_type const& result) {
    auto&& stmt = *result.value()->statements().at(0);
    auto&& select = downcast<statement::select_statement>(stmt);
    return *select.expression();
}

TEST_F(sql_parser_query_test, query_asterisk) {
    sql_parser parser;
    auto result = parser("-", "SELECT * FROM T0;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (query::query{
            {},
            { // SELECT
                    query::select_asterisk{},
            },
            { // FROM
                    table::table_reference{
                            name::simple{"T0"},
                    },
            },
            { // WHERE
            }
    }));
}

TEST_F(sql_parser_query_test, query_asterisk_qualified) {
    sql_parser parser;
    auto result = parser("-", "SELECT T0.* FROM T0;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (query::query {
            {},
            { // SELECT
                    query::select_asterisk {
                            scalar::variable_reference {
                                    name::simple { "T0" },
                            },
                    },
            },
            { // FROM
                    table::table_reference {
                            name::simple { "T0" },
                    },
            },
            { // WHERE
            }
    }));
}

TEST_F(sql_parser_query_test, query_column) {
    sql_parser parser;
    auto result = parser("-", "SELECT C0 FROM T0;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (query::query {
            {},
            { // SELECT
                    query::select_column {
                            scalar::variable_reference {
                                    name::simple { "C0" },
                            },
                    },
            },
            { // FROM
                    table::table_reference {
                            name::simple { "T0" },
                    },
            },
            { // WHERE
            }
    }));
}

TEST_F(sql_parser_query_test, query_column_name) {
    sql_parser parser;
    auto result = parser("-", "SELECT C0 AS x0 FROM T0;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (query::query {
            {},
            { // SELECT
                    query::select_column {
                            scalar::variable_reference {
                                    name::simple { "C0" },
                            },
                            name::simple { "x0" },
                    },
            },
            { // FROM
                    table::table_reference {
                            name::simple { "T0" },
                    },
            },
            { // WHERE
            }
    }));
}

TEST_F(sql_parser_query_test, query_where) {
    sql_parser parser;
    auto result = parser("-", "SELECT * FROM T0 WHERE a < 0;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (query::query {
            {},
            { // SELECT
                    query::select_asterisk {},
            },
            { // FROM
                    table::table_reference {
                            name::simple { "T0" },
                    },
            },
            { // WHERE
                    scalar::comparison_predicate {
                            scalar::variable_reference {
                                    name::simple { "a" },
                            },
                            scalar::comparison_operator::less_than,
                            int_literal("0"),
                    },
            }
    }));
}

TEST_F(sql_parser_query_test, query_group_by_grand_total) {
    sql_parser parser;
    auto result = parser("-", "SELECT C0, C1 FROM T0 GROUP BY ();");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (query::query {
            {},
            { // SELECT
                    query::select_column {
                            scalar::variable_reference {
                                    name::simple { "C0" },
                            },
                    },
                    query::select_column {
                            scalar::variable_reference {
                                    name::simple { "C1" },
                            },
                    },
            },
            { // FROM
                    table::table_reference {
                            name::simple { "T0" },
                    },
            },
            { // WHERE
            },
            { // GROUP BY
                    {}, // grand total
            },
    }));
}

TEST_F(sql_parser_query_test, query_group_by_columns) {
    sql_parser parser;
    auto result = parser("-", "SELECT C0, C1 FROM T0 GROUP BY C0, C1;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (query::query {
            {},
            { // SELECT
                    query::select_column {
                            scalar::variable_reference {
                                    name::simple { "C0" },
                            },
                    },
                    query::select_column {
                            scalar::variable_reference {
                                    name::simple { "C1" },
                            },
                    },
            },
            { // FROM
                    table::table_reference {
                            name::simple { "T0" },
                    },
            },
            { // WHERE
            },
            { // GROUP BY
                    {
                            query::grouping_column {
                                    scalar::variable_reference {
                                            name::simple { "C0" },
                                    },
                            },
                            query::grouping_column {
                                    scalar::variable_reference {
                                            name::simple { "C1" },
                                    },
                            },
                    },
            },
    }));
}

TEST_F(sql_parser_query_test, query_group_by_having) {
    sql_parser parser;
    auto result = parser("-", "SELECT C0 FROM T0 GROUP BY C0, C1 HAVING C1 = 0;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (query::query {
            {},
            { // SELECT
                    query::select_column {
                            scalar::variable_reference {
                                    name::simple { "C0" },
                            },
                    },
            },
            { // FROM
                    table::table_reference {
                            name::simple { "T0" },
                    },
            },
            { // WHERE
            },
            { // GROUP BY
                    {
                            query::grouping_column {
                                    scalar::variable_reference {
                                            name::simple { "C0" },
                                    },
                            },
                            query::grouping_column {
                                    scalar::variable_reference {
                                            name::simple { "C1" },
                                    },
                            },
                    },
            },
            { // HAVING
                    scalar::comparison_predicate {
                            scalar::variable_reference {
                                    name::simple { "C1" },
                            },
                            scalar::comparison_operator::equals,
                            int_literal("0"),
                    },
            },
    }));
}

TEST_F(sql_parser_query_test, query_order_by) {
    sql_parser parser;
    auto result = parser("-", "SELECT C0 FROM T0 ORDER BY C0, C1 ASC, C2 DESC;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (query::query {
            {},
            { // SELECT
                    query::select_column {
                            scalar::variable_reference {
                                    name::simple { "C0" },
                            },
                    },
            },
            { // FROM
                    table::table_reference {
                            name::simple { "T0" },
                    },
            },
            { // WHERE
            },
            { // GROUP BY
            },
            { // HAVING
            },
            { // ORDER BY
                    name::simple { "C0" },
                    {
                            name::simple { "C1" },
                            common::ordering_specification::asc,
                    },
                    {
                            name::simple { "C2" },
                            common::ordering_specification::desc,
                    },
            },
    }));
}

TEST_F(sql_parser_query_test, query_limit) {
    sql_parser parser;
    auto result = parser("-", "SELECT C0 FROM T0 LIMIT 10;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (query::query {
            {},
            { // SELECT
                    query::select_column {
                            scalar::variable_reference {
                                    name::simple { "C0" },
                            },
                    },
            },
            { // FROM
                    table::table_reference {
                            name::simple { "T0" },
                    },
            },
            { // WHERE
            },
            { // GROUP BY
            },
            { // HAVING
            },
            { // ORDER BY
            },
            int_literal("10"),
    }));
}

} // namespace mizugaki::parser
