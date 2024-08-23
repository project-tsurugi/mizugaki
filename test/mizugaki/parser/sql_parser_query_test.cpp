#include <mizugaki/parser/sql_parser.h>

#include <gtest/gtest.h>

#include <mizugaki/ast/statement/select_statement.h>

#include <mizugaki/ast/query/query.h>
#include <mizugaki/ast/query/table_reference.h>
#include <mizugaki/ast/query/table_value_constructor.h>
#include <mizugaki/ast/query/binary_expression.h>
#include <mizugaki/ast/query/with_expression.h>
#include <mizugaki/ast/query/select_asterisk.h>
#include <mizugaki/ast/query/select_column.h>
#include <mizugaki/ast/query/grouping_column.h>

#include <mizugaki/ast/table/table_reference.h>

#include <mizugaki/ast/scalar/literal_expression.h>
#include <mizugaki/ast/scalar/variable_reference.h>
#include <mizugaki/ast/scalar/comparison_predicate.h>
#include <mizugaki/ast/scalar/value_constructor.h>

#include <mizugaki/ast/name/simple.h>

#include "utils.h"

namespace mizugaki::parser {

using namespace testing;

class sql_parser_query_test : public ::testing::Test {
protected:
    sql_parser_result parse(std::string_view str, sql_parser parser = default_parser()) {
        using ::takatori::util::string_builder;
        return parser("-",std::string { str });
    }
};

static query::expression const& extract(sql_parser::result_type const& result) {
    auto&& stmt = *result.value()->statements().at(0);
    auto&& select = downcast<statement::select_statement>(stmt);
    return *select.expression();
}

TEST_F(sql_parser_query_test, query_asterisk) {
    auto result = parse("SELECT * FROM T0;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (query::query {
            { // SELECT
                    query::select_asterisk{},
            },
            { // FROM
                    table::table_reference {
                            name::simple { "T0" },
                    },
            },
    }));
}

TEST_F(sql_parser_query_test, query_distinct) {
    auto result = parse("SELECT DISTINCT * FROM T0;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (query::query {
            query::set_quantifier::distinct,
            { // SELECT
                    query::select_asterisk{},
            },
            { // FROM
                    table::table_reference {
                            name::simple { "T0" },
                    },
            },
    }));
}

TEST_F(sql_parser_query_test, query_asterisk_qualified) {
    auto result = parse("SELECT T0.* FROM T0;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (query::query {
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
    }));
}

TEST_F(sql_parser_query_test, query_column) {
    auto result = parse("SELECT C0 FROM T0;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (query::query {
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
    }));
}

TEST_F(sql_parser_query_test, query_column_name) {
    auto result = parse("SELECT C0 AS x0 FROM T0;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (query::query {
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
    }));
}

TEST_F(sql_parser_query_test, query_column_name_without_as) {
    auto result = parse("SELECT C0 x0 FROM T0;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (query::query {
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
    }));
}

TEST_F(sql_parser_query_test, query_where) {
    auto result = parse("SELECT * FROM T0 WHERE a < 0;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (query::query {
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
            },
    }));
}

TEST_F(sql_parser_query_test, query_group_by_grand_total) {
    auto result = parse("SELECT C0, C1 FROM T0 GROUP BY ();");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (query::query {
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
    auto result = parse("SELECT C0, C1 FROM T0 GROUP BY C0, C1;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (query::query {
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
    auto result = parse("SELECT C0 FROM T0 GROUP BY C0, C1 HAVING C1 = 0;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (query::query {
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
    auto result = parse("SELECT C0 FROM T0 ORDER BY C0, C1 ASC, C2 DESC;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (query::query {
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
    auto result = parse("SELECT C0 FROM T0 LIMIT 10;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (query::query {
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

TEST_F(sql_parser_query_test, query_wo_from) {
    auto result = parse("SELECT 1;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (query::query {
            { // SELECT
                    query::select_column {
                            int_literal("1"),
                    },
            },
    }));
}

TEST_F(sql_parser_query_test, explicit_table) {
    auto result = parse("TABLE T0;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (query::table_reference {
            name::simple { "T0" },
    }));
}

TEST_F(sql_parser_query_test, table_value_constructor) {
    auto result = parse("VALUES (1);");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (query::table_value_constructor {
            scalar::value_constructor {
                    int_literal("1"),
            },
    }));
}

TEST_F(sql_parser_query_test, table_value_constructor_columns) {
    auto result = parse("VALUES (1, 2, 3);");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (query::table_value_constructor {
            scalar::value_constructor {
                    int_literal("1"),
                    int_literal("2"),
                    int_literal("3"),
            },
    }));
}

TEST_F(sql_parser_query_test, table_value_constructor_rows) {
    auto result = parse("VALUES (1), (2), (3);");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (query::table_value_constructor {
            scalar::value_constructor {
                    int_literal("1"),
            },
            scalar::value_constructor {
                    int_literal("2"),
            },
            scalar::value_constructor {
                    int_literal("3"),
            },
    }));
}

TEST_F(sql_parser_query_test, binary_expression_union) {
    auto result = parse("TABLE a UNION TABLE b;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (query::binary_expression {
            query::table_reference {
                    name::simple { "a" },
            },
            query::binary_operator::union_,
            query::table_reference {
                    name::simple { "b" },
            },
    }));
}

TEST_F(sql_parser_query_test, binary_expression_outer_union) {
    auto result = parse("TABLE a OUTER UNION TABLE b;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (query::binary_expression {
            query::table_reference {
                    name::simple { "a" },
            },
            query::binary_operator::outer_union,
            query::table_reference {
                    name::simple { "b" },
            },
    }));
}

TEST_F(sql_parser_query_test, binary_expression_except) {
    auto result = parse("TABLE a EXCEPT TABLE b;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (query::binary_expression {
            query::table_reference {
                    name::simple { "a" },
            },
            query::binary_operator::except,
            query::table_reference {
                    name::simple { "b" },
            },
    }));
}

TEST_F(sql_parser_query_test, binary_expression_intersect) {
    auto result = parse("TABLE a INTERSECT TABLE b;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (query::binary_expression {
            query::table_reference {
                    name::simple { "a" },
            },
            query::binary_operator::intersect,
            query::table_reference {
                    name::simple { "b" },
            },
    }));
}

TEST_F(sql_parser_query_test, binary_expression_precedence) {
    auto result = parse("TABLE T0 UNION TABLE T1 EXCEPT TABLE T2 INTERSECT TABLE T3;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (query::binary_expression {
            query::binary_expression {
                    query::table_reference {
                            name::simple { "T0" },
                    },
                    query::binary_operator::union_,
                    query::table_reference {
                            name::simple { "T1" },
                    },
            },
            query::binary_operator::except,
            query::binary_expression {
                    query::table_reference {
                            name::simple { "T2" },
                    },
                    query::binary_operator::intersect,
                    query::table_reference {
                            name::simple { "T3" },
                    },
            },
    }));
}

TEST_F(sql_parser_query_test, binary_expression_all) {
    auto result = parse("TABLE a UNION ALL TABLE b;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (query::binary_expression {
            query::table_reference {
                    name::simple { "a" },
            },
            query::binary_operator::union_,
            query::set_quantifier::all,
            query::table_reference {
                    name::simple { "b" },
            },
    }));
}

TEST_F(sql_parser_query_test, binary_expression_distinct) {
    auto result = parse("TABLE a UNION DISTINCT TABLE b;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (query::binary_expression {
            query::table_reference {
                    name::simple { "a" },
            },
            query::binary_operator::union_,
            query::set_quantifier::distinct,
            query::table_reference {
                    name::simple { "b" },
            },
    }));
}

TEST_F(sql_parser_query_test, binary_expression_corresponding) {
    auto result = parse("TABLE a UNION ALL CORRESPONDING TABLE b;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (query::binary_expression {
            query::table_reference {
                    name::simple { "a" },
            },
            query::binary_operator::union_,
            query::set_quantifier::all,
            {},
            query::table_reference {
                    name::simple { "b" },
            },
    }));
}

TEST_F(sql_parser_query_test, binary_expression_corresponding_by) {
    auto result = parse("TABLE a UNION ALL CORRESPONDING BY (C0, C1, C2) TABLE b;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (query::binary_expression {
            query::table_reference {
                    name::simple { "a" },
            },
            query::binary_operator::union_,
            query::set_quantifier::all,
            {
                    name::simple { "C0" },
                    name::simple { "C1" },
                    name::simple { "C2" },
            },
            query::table_reference {
                    name::simple { "b" },
            },
    }));
}

TEST_F(sql_parser_query_test, with_expression) {
    auto result = parse("WITH q AS (SELECT * FROM T0) SELECT * FROM q;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (query::with_expression {
            { // WITH
                    {
                            name::simple { "q" },
                            query::query {
                                    { // SELECT
                                            query::select_asterisk {},
                                    },
                                    { // FROM
                                            table::table_reference {
                                                    name::simple { "T0" },
                                            },
                                    },
                                    { // WHERE
                                    }
                            }
                    },
            },
            query::query {
                    { // SELECT
                            query::select_asterisk {},
                    },
                    { // FROM
                            table::table_reference {
                                    name::simple { "q" },
                            },
                    },
            }
    }));
}

TEST_F(sql_parser_query_test, with_expression_recursive) {
    auto result = parse("WITH RECURSIVE q AS (SELECT * FROM T0) SELECT * FROM q;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (query::with_expression {
            true,
            { // WITH
                    {
                            name::simple { "q" },
                            query::query {
                                    { // SELECT
                                            query::select_asterisk {},
                                    },
                                    { // FROM
                                            table::table_reference {
                                                    name::simple { "T0" },
                                            },
                                    },
                                    { // WHERE
                                    }
                            }
                    },
            },
            query::query {
                    { // SELECT
                            query::select_asterisk {},
                    },
                    { // FROM
                            table::table_reference {
                                    name::simple { "q" },
                            },
                    },
            }
    }));
}

TEST_F(sql_parser_query_test, with_expression_columns) {
    auto result = parse("WITH q (x, y) AS (SELECT * FROM T0) SELECT * FROM q;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (query::with_expression {
            { // WITH
                    {
                            name::simple { "q" },
                            {
                                    name::simple { "x" },
                                    name::simple { "y" },
                            },
                            query::query {
                                    { // SELECT
                                            query::select_asterisk {},
                                    },
                                    { // FROM
                                            table::table_reference {
                                                    name::simple { "T0" },
                                            },
                                    },
                                    { // WHERE
                                    }
                            }
                    },
            },
            query::query {
                    { // SELECT
                            query::select_asterisk {},
                    },
                    { // FROM
                            table::table_reference {
                                    name::simple { "q" },
                            },
                    },
            }
    }));
}

TEST_F(sql_parser_query_test, with_expression_multiple_elements) {
    auto result = parse(
            "WITH q AS (SELECT * FROM T0)"
            ",  r AS (SELECT * FROM q)"
            ",  s AS (SELECT * FROM r)"
            "SELECT * FROM s;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (query::with_expression {
            { // WITH
                    {
                            name::simple { "q" },
                            query::query {
                                    { // SELECT
                                            query::select_asterisk {},
                                    },
                                    { // FROM
                                            table::table_reference {
                                                    name::simple { "T0" },
                                            },
                                    },
                            }
                    },
                    {
                            name::simple { "r" },
                            query::query {
                                    { // SELECT
                                            query::select_asterisk {},
                                    },
                                    { // FROM
                                            table::table_reference {
                                                    name::simple { "q" },
                                            },
                                    },
                            }
                    },
                    {
                            name::simple { "s" },
                            query::query {
                                    { // SELECT
                                            query::select_asterisk {},
                                    },
                                    { // FROM
                                            table::table_reference {
                                                    name::simple { "r" },
                                            },
                                    },
                            }
                    },
            },
            query::query {
                    { // SELECT
                            query::select_asterisk {},
                    },
                    { // FROM
                            table::table_reference {
                                    name::simple { "s" },
                            },
                    },
            }
    }));
}

TEST_F(sql_parser_query_test, parenthesize) {
    auto result = parse("TABLE T0 UNION (TABLE T1 EXCEPT TABLE T2);");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (query::binary_expression {
            query::table_reference {
                    name::simple { "T0" },
            },
            query::binary_operator::union_,
            query::binary_expression {
                    query::table_reference {
                            name::simple { "T1" },
                    },
                    query::binary_operator::except,
                    query::table_reference {
                            name::simple { "T2" },
                    },
            },
    }));
}

} // namespace mizugaki::parser
