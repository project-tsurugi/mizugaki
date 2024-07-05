#include <mizugaki/analyzer/details/analyze_query_expression.h>

#include <gtest/gtest.h>

#include <takatori/type/primitive.h>

#include <takatori/relation/scan.h>
#include <takatori/relation/values.h>

#include <yugawara/binding/extract.h>

#include <mizugaki/ast/scalar/value_constructor.h>

#include <mizugaki/ast/query/table_reference.h>
#include <mizugaki/ast/query/table_value_constructor.h>

#include "test_parent.h"

namespace mizugaki::analyzer::details {

using namespace ::mizugaki::analyzer::testing;

using ::yugawara::binding::extract;

class analyze_query_expression_test : public test_parent {};

TEST_F(analyze_query_expression_test, table_reference) {
    auto table = install_table("testing");
    trelation::graph_type graph {};

    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::table_reference {
                    id("testing"),
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(graph.size(), 1);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();
    ASSERT_TRUE(relation.declaration());
    EXPECT_EQ(downcast<::yugawara::storage::table>(*relation.declaration()), *table);

    // FIXME: check spec
    // EXPECT_EQ(relation.identifier(), "testing");

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 4);
    {
        auto&& column = relation_columns[0];
        EXPECT_EQ(column.identifier(), "k");
        EXPECT_EQ(column.declaration(), table->columns()[0]);
        EXPECT_TRUE(column.exported());
    }
    {
        auto&& column = relation_columns[1];
        EXPECT_EQ(column.identifier(), "v");
        EXPECT_EQ(column.declaration(), table->columns()[1]);
        EXPECT_TRUE(column.exported());
    }
    {
        auto&& column = relation_columns[2];
        EXPECT_EQ(column.identifier(), "w");
        EXPECT_EQ(column.declaration(), table->columns()[2]);
        EXPECT_TRUE(column.exported());
    }
    {
        auto&& column = relation_columns[3];
        EXPECT_EQ(column.identifier(), "x");
        EXPECT_EQ(column.declaration(), table->columns()[3]);
        EXPECT_TRUE(column.exported());
    }

    // scan -
    auto&& scan = downcast<trelation::scan>(r.output().owner());
    EXPECT_EQ(extract<::yugawara::storage::index>(scan.source()).table(), *table);

    auto&& scan_columns = scan.columns();
    ASSERT_EQ(scan_columns.size(), 4);
    {
        auto&& column = scan_columns[0];
        EXPECT_EQ(extract<::yugawara::storage::column>(column.source()), table->columns()[0]);
        EXPECT_EQ(column.destination(), relation.columns()[0].variable());
    }
    {
        auto&& column = scan_columns[1];
        EXPECT_EQ(extract<::yugawara::storage::column>(column.source()), table->columns()[1]);
        EXPECT_EQ(column.destination(), relation.columns()[1].variable());
    }
    {
        auto&& column = scan_columns[2];
        EXPECT_EQ(extract<::yugawara::storage::column>(column.source()), table->columns()[2]);
        EXPECT_EQ(column.destination(), relation.columns()[2].variable());
    }
    {
        auto&& column = scan_columns[3];
        EXPECT_EQ(extract<::yugawara::storage::column>(column.source()), table->columns()[3]);
        EXPECT_EQ(column.destination(), relation.columns()[3].variable());
    }

    EXPECT_EQ(scan.lower(), (trelation::scan::endpoint {}));
    EXPECT_EQ(scan.upper(), (trelation::scan::endpoint {}));
    EXPECT_EQ(scan.limit(), std::nullopt);
}

TEST_F(analyze_query_expression_test, table_value_constructor) {
    trelation::graph_type graph {};
    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::table_value_constructor {
                    ast::scalar::value_constructor {
                            literal(number("1")),
                            literal(number("2")),
                            literal(number("3")),
                    },
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(graph.size(), 1);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();
    EXPECT_FALSE(relation.declaration());
    EXPECT_EQ(relation.identifier(), "");

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 3);
    {
        auto&& column = relation_columns[0];
        EXPECT_EQ(column.identifier(), "");
        EXPECT_FALSE(column.declaration());
        EXPECT_TRUE(column.exported());
    }
    {
        auto&& column = relation_columns[1];
        EXPECT_EQ(column.identifier(), "");
        EXPECT_FALSE(column.declaration());
        EXPECT_TRUE(column.exported());
    }
    {
        auto&& column = relation_columns[2];
        EXPECT_EQ(column.identifier(), "");
        EXPECT_FALSE(column.declaration());
        EXPECT_TRUE(column.exported());
    }

    // values -
    auto&& values = downcast<trelation::values>(r.output().owner());
    auto&& values_columns = values.columns();
    ASSERT_EQ(values_columns.size(), 3);
    EXPECT_EQ(values_columns[0], relation_columns[0].variable());
    EXPECT_EQ(values_columns[1], relation_columns[1].variable());
    EXPECT_EQ(values_columns[2], relation_columns[2].variable());

    auto&& values_rows = values.rows();
    ASSERT_EQ(values_rows.size(), 1);
    {
        auto&& row = values_rows[0].elements();
        ASSERT_EQ(row.size(), 3);
        EXPECT_EQ(row[0], immediate(1));
        EXPECT_EQ(row[1], immediate(2));
        EXPECT_EQ(row[2], immediate(3));
    }
}

TEST_F(analyze_query_expression_test, table_value_constructor_multiple_rows) {
    trelation::graph_type graph {};
    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::table_value_constructor {
                    ast::scalar::value_constructor {
                            literal(number("1")),
                            literal(number("2")),
                    },
                    ast::scalar::value_constructor {
                            literal(number("3")),
                            literal(number("4")),
                    },
                    ast::scalar::value_constructor {
                            literal(number("5")),
                            literal(number("6")),
                    },
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(graph.size(), 1);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();
    EXPECT_FALSE(relation.declaration());
    EXPECT_EQ(relation.identifier(), "");

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 2);

    // values -
    auto&& values = downcast<trelation::values>(r.output().owner());
    auto&& values_columns = values.columns();
    ASSERT_EQ(values_columns.size(), 2);

    auto&& values_rows = values.rows();
    ASSERT_EQ(values_rows.size(), 3);
    {
        auto&& row = values_rows[0].elements();
        ASSERT_EQ(row.size(), 2);
        EXPECT_EQ(row[0], immediate(1));
        EXPECT_EQ(row[1], immediate(2));
    }
    {
        auto&& row = values_rows[1].elements();
        ASSERT_EQ(row.size(), 2);
        EXPECT_EQ(row[0], immediate(3));
        EXPECT_EQ(row[1], immediate(4));
    }
    {
        auto&& row = values_rows[2].elements();
        ASSERT_EQ(row.size(), 2);
        EXPECT_EQ(row[0], immediate(5));
        EXPECT_EQ(row[1], immediate(6));
    }
}

} // namespace mizugaki::analyzer::details
