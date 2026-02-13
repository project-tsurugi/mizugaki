#include <mizugaki/analyzer/details/analyze_query_expression.h>

#include <gtest/gtest.h>

#include <takatori/type/primitive.h>

#include <takatori/relation/values.h>
#include <takatori/relation/filter.h>
#include <takatori/relation/project.h>

#include <mizugaki/ast/table/subquery.h>

#include <mizugaki/ast/query/query.h>
#include <mizugaki/ast/query/select_column.h>
#include <mizugaki/ast/query/select_asterisk.h>

#include "test_parent.h"

namespace mizugaki::analyzer::details {

using namespace ::mizugaki::analyzer::testing;

class analyze_query_expression_subquery_test : public test_parent {
protected:
    void invalid(ast::query::expression const& expression) {
        trelation::graph_type graph {};
        auto r = analyze_query_expression(
                context(),
                graph,
                expression,
                {},
                {});
        EXPECT_FALSE(r);
        EXPECT_NE(count_error(), 0);
    }

    void invalid(sql_analyzer_code code, ast::query::expression const& expression) {
        invalid(expression);
        EXPECT_TRUE(find_error(code)) << diagnostics();
    }
};

TEST_F(analyze_query_expression_subquery_test, simple) {
    trelation::graph_type graph {};
    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::query {
                    {
                            ast::query::select_asterisk {},
                    },
                    {
                            ast::table::subquery {
                                    ast::query::query {
                                            {
                                                    ast::query::select_column {
                                                            literal(number("1")),
                                                            id("V"),
                                                    },
                                            },
                                    },
                                    {
                                            id("Q"),
                                    },
                            },
                    },
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(graph.size(), 3);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();
    EXPECT_EQ(relation.identifier(), "");

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 1);
    EXPECT_EQ(relation_columns[0].identifier(), "v");

    // values - project - project -
    auto&& outer = downcast<trelation::project>(r.output().owner());
    auto&& inner = *find_prev<trelation::project>(outer);
    auto&& values = *find_prev<trelation::values>(inner);

    auto&& values_columns = values.columns();
    ASSERT_EQ(values_columns.size(), 0);

    auto&& values_rows = values.rows();
    ASSERT_EQ(values_rows.size(), 1);
    EXPECT_EQ(values_rows[0].elements().size(), 0);

    auto&& inner_columns = inner.columns();
    ASSERT_EQ(inner_columns.size(), 1);
    EXPECT_EQ(inner_columns[0].value(), immediate(1));

    auto&& outer_columns = outer.columns();
    ASSERT_EQ(outer_columns.size(), 0);

    EXPECT_EQ(relation_columns[0].variable(), inner_columns[0].variable());
}

TEST_F(analyze_query_expression_subquery_test, correlation_column) {
    trelation::graph_type graph {};
    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::query {
                    {
                            ast::query::select_column {
                                    vref(ast::name::qualified(id("Q"), id("V"))),
                            },
                    },
                    {
                            ast::table::subquery {
                                    ast::query::query {
                                            {
                                                    ast::query::select_column {
                                                            literal(number("1")),
                                                    },
                                            },
                                    },
                                    {
                                            id("Q"),
                                            {
                                                    id("V"),
                                            },
                                    },
                            },
                    },
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(graph.size(), 3);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();
    EXPECT_EQ(relation.identifier(), "");

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 1);
    EXPECT_EQ(relation_columns[0].identifier(), "v");

    // values - project - project -
    auto&& outer = downcast<trelation::project>(r.output().owner());
    auto&& inner = *find_prev<trelation::project>(outer);
    auto&& values = *find_prev<trelation::values>(inner);

    auto&& values_columns = values.columns();
    ASSERT_EQ(values_columns.size(), 0);

    auto&& values_rows = values.rows();
    ASSERT_EQ(values_rows.size(), 1);
    EXPECT_EQ(values_rows[0].elements().size(), 0);

    auto&& inner_columns = inner.columns();
    ASSERT_EQ(inner_columns.size(), 1);
    EXPECT_EQ(inner_columns[0].value(), immediate(1));

    auto&& outer_columns = outer.columns();
    ASSERT_EQ(outer_columns.size(), 1);
    EXPECT_EQ(outer_columns[0].value(), vref(inner_columns[0].variable()));

    EXPECT_EQ(relation_columns[0].variable(), outer_columns[0].variable());
}

TEST_F(analyze_query_expression_subquery_test, correlation_shrink_columns) {
    trelation::graph_type graph {};
    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::query {
                    {
                            ast::query::select_asterisk {},
                    },
                    {
                            ast::table::subquery {
                                    ast::query::query {
                                            {
                                                    ast::query::select_column {
                                                            literal(number("1")),
                                                    },
                                                    ast::query::select_column {
                                                            literal(number("2")),
                                                    },
                                                    ast::query::select_column {
                                                            literal(number("3")),
                                                    },
                                            },
                                    },
                                    {
                                            id("Q"),
                                            {
                                                    id("V"),
                                                    id("W"),
                                            },
                                    },
                            },
                    },
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(graph.size(), 3);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();
    EXPECT_EQ(relation.identifier(), "");

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 2);
    EXPECT_EQ(relation_columns[0].identifier(), "v");
    EXPECT_EQ(relation_columns[1].identifier(), "w");

    // values - project - project -
    auto&& outer = downcast<trelation::project>(r.output().owner());
    auto&& inner = *find_prev<trelation::project>(outer);
    auto&& values = *find_prev<trelation::values>(inner);

    auto&& values_columns = values.columns();
    ASSERT_EQ(values_columns.size(), 0);

    auto&& values_rows = values.rows();
    ASSERT_EQ(values_rows.size(), 1);
    EXPECT_EQ(values_rows[0].elements().size(), 0);

    auto&& inner_columns = inner.columns();
    ASSERT_EQ(inner_columns.size(), 3);
    EXPECT_EQ(inner_columns[0].value(), immediate(1));
    EXPECT_EQ(inner_columns[1].value(), immediate(2));
    EXPECT_EQ(inner_columns[2].value(), immediate(3));

    auto&& outer_columns = outer.columns();
    ASSERT_EQ(outer_columns.size(), 0);

    EXPECT_EQ(relation_columns[0].variable(), inner_columns[0].variable());
    EXPECT_EQ(relation_columns[1].variable(), inner_columns[1].variable());
}

TEST_F(analyze_query_expression_subquery_test, invalid_query) {
    invalid(ast::query::query {
            {
                    ast::query::select_asterisk {},
            },
            {
                    ast::table::subquery {
                            ast::query::query {
                                    {
                                            ast::query::select_column {
                                                    vref(id("MISSING")),
                                                    id("V"),
                                            },
                                    },
                            },
                            {
                                    id("Q"),
                            },
                    },
            },
    });
}

TEST_F(analyze_query_expression_subquery_test, invalid_correlation_too_many) {
    invalid(sql_analyzer_code::inconsistent_columns, ast::query::query {
            {
                    ast::query::select_asterisk {},
            },
            {
                    ast::table::subquery {
                            ast::query::query {
                                    {
                                            ast::query::select_column {
                                                    literal(number("1")),
                                            },
                                    },
                            },
                            {
                                    id("Q"),
                                    {
                                            id("V"),
                                            id("OVER"),
                                    },
                            },
                    },
            },
    });
}

TEST_F(analyze_query_expression_subquery_test, invalid_correlation_column_conflict) {
    invalid(sql_analyzer_code::column_already_exists, ast::query::query {
            {
                    ast::query::select_asterisk {},
            },
            {
                    ast::table::subquery {
                            ast::query::query {
                                    {
                                            ast::query::select_column {
                                                    literal(number("1")),
                                            },
                                            ast::query::select_column {
                                                    literal(number("2")),
                                            },
                                            ast::query::select_column {
                                                    literal(number("3")),
                                            },
                                    },
                            },
                            {
                                    id("Q"),
                                    {
                                            id("c0"),
                                            id("c1"),
                                            id("C0"), // conflict column name
                                    },
                            },
                    },
            },
    });
}

} // namespace mizugaki::analyzer::details
