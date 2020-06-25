#include <mizugaki/analyzer/details/analyze_query_expression.h>

#include <gtest/gtest.h>

#include <takatori/type/primitive.h>

#include <takatori/scalar/compare.h>

#include <takatori/relation/scan.h>
#include <takatori/relation/filter.h>
#include <takatori/relation/project.h>

#include <yugawara/binding/extract.h>

#include <mizugaki/ast/name/qualified.h>

#include <mizugaki/ast/scalar/comparison_predicate.h>

#include <mizugaki/ast/table/table_reference.h>

#include <mizugaki/ast/query/query.h>
#include <mizugaki/ast/query/select_asterisk.h>
#include <mizugaki/ast/query/select_column.h>

#include "test_parent.h"

namespace mizugaki::analyzer::details {

using namespace ::mizugaki::analyzer::testing;

using ::yugawara::binding::extract;

class analyze_query_expression_select_test : public test_parent {};

TEST_F(analyze_query_expression_select_test, simple) {
    auto table = install_table("testing");
    trelation::graph_type graph {};

    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::query {
                    {
                            ast::query::select_column { vref(id("v")) },
                    },
                    {
                            ast::table::table_reference {
                                    id("testing"),
                            }
                    },
            },
            {},
            {});
    ASSERT_TRUE(r);
    EXPECT_EQ(graph.size(), 2);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();
    EXPECT_EQ(relation.identifier(), "");

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 1);
    {
        auto&& column = relation_columns[0];
        // FIXME: check spec
        // EXPECT_EQ(column.identifier(), "v");
        EXPECT_TRUE(column.exported());
    }

    // scan - project -
    auto&& project = downcast<trelation::project>(r.output().owner());
    auto&& scan = *find_prev<trelation::scan>(project);

    EXPECT_EQ(extract<::yugawara::storage::index>(scan.source()).table(), *table);

    auto&& scan_columns = scan.columns();
    ASSERT_EQ(scan_columns.size(), 4);

    auto&& project_columns = project.columns();
    ASSERT_EQ(project_columns.size(), 1);
    {
        auto&& column = project_columns[0];
        EXPECT_EQ(column.value(), vref(scan_columns[1].destination())); // v
    }

    EXPECT_EQ(relation_columns[0].variable(), project_columns[0].variable());
}

TEST_F(analyze_query_expression_select_test, select_asterisk_simple) {
    auto table = install_table("testing");
    trelation::graph_type graph {};

    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::query {
                    {
                            ast::query::select_asterisk {},
                    },
                    {
                            ast::table::table_reference {
                                    id("testing"),
                            }
                    },
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    EXPECT_EQ(graph.size(), 2);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();
    EXPECT_EQ(relation.identifier(), "");

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

    // scan - project -
    auto&& project = downcast<trelation::project>(r.output().owner());
    auto&& scan = *find_prev<trelation::scan>(project);

    EXPECT_EQ(extract<::yugawara::storage::index>(scan.source()).table(), *table);

    auto&& scan_columns = scan.columns();
    ASSERT_EQ(scan_columns.size(), 4);

    auto&& project_columns = project.columns();
    ASSERT_EQ(project_columns.size(), 0);

    EXPECT_EQ(relation_columns[0].variable(), scan_columns[0].destination());
    EXPECT_EQ(relation_columns[1].variable(), scan_columns[1].destination());
    EXPECT_EQ(relation_columns[2].variable(), scan_columns[2].destination());
    EXPECT_EQ(relation_columns[3].variable(), scan_columns[3].destination());
}

TEST_F(analyze_query_expression_select_test, select_multiple_select_element) {
    auto table = install_table("testing");
    trelation::graph_type graph {};

    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::query {
                    {
                            ast::query::select_column { vref(id("w")) },
                            ast::query::select_column { vref(id("x")) },
                            ast::query::select_column { vref(id("v")) },
                    },
                    {
                            ast::table::table_reference {
                                    id("testing"),
                            }
                    },
            },
            {},
            {});
    ASSERT_TRUE(r);
    EXPECT_EQ(graph.size(), 2);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();
    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 3);

    // scan - project -
    auto&& project = downcast<trelation::project>(r.output().owner());
    auto&& scan = *find_prev<trelation::scan>(project);

    auto&& scan_columns = scan.columns();
    ASSERT_EQ(scan_columns.size(), 4);

    auto&& project_columns = project.columns();
    ASSERT_EQ(project_columns.size(), 3);
    {
        auto&& column = project_columns[0];
        EXPECT_EQ(column.value(), vref(scan_columns[2].destination())); // w
    }
    {
        auto&& column = project_columns[1];
        EXPECT_EQ(column.value(), vref(scan_columns[3].destination())); // x
    }
    {
        auto&& column = project_columns[2];
        EXPECT_EQ(column.value(), vref(scan_columns[1].destination())); // v
    }

    EXPECT_EQ(relation_columns[0].variable(), project_columns[0].variable());
    EXPECT_EQ(relation_columns[1].variable(), project_columns[1].variable());
    EXPECT_EQ(relation_columns[2].variable(), project_columns[2].variable());
}

TEST_F(analyze_query_expression_select_test, select_relation_name) {
    auto table = install_table("testing");
    trelation::graph_type graph {};

    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::query {
                    {
                            ast::query::select_column { vref(ast::name::qualified(id("testing"), id("v"))) },
                    },
                    {
                            ast::table::table_reference {
                                    id("testing"),
                            }
                    },
            },
            {},
            {});
    ASSERT_TRUE(r);
    EXPECT_EQ(graph.size(), 2);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();
    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 1);

    // scan - project -
    auto&& project = downcast<trelation::project>(r.output().owner());
    auto&& scan = *find_prev<trelation::scan>(project);

    auto&& scan_columns = scan.columns();
    ASSERT_EQ(scan_columns.size(), 4);

    auto&& project_columns = project.columns();
    ASSERT_EQ(project_columns.size(), 1);
    {
        auto&& column = project_columns[0];
        EXPECT_EQ(column.value(), vref(scan_columns[1].destination())); // v
    }

    EXPECT_EQ(relation_columns[0].variable(), project_columns[0].variable());
}

TEST_F(analyze_query_expression_select_test, select_corelation_name) {
    auto table = install_table("testing");
    trelation::graph_type graph {};

    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::query {
                    {
                            ast::query::select_column { vref(ast::name::qualified(id("alter"), id("v"))) },
                    },
                    {
                            ast::table::table_reference {
                                    id("testing"),
                                    id("alter"),
                            }
                    },
            },
            {},
            {});
    ASSERT_TRUE(r);
    EXPECT_EQ(graph.size(), 2);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();
    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 1);

    // scan - project -
    auto&& project = downcast<trelation::project>(r.output().owner());
    auto&& scan = *find_prev<trelation::scan>(project);

    auto&& scan_columns = scan.columns();
    ASSERT_EQ(scan_columns.size(), 4);

    auto&& project_columns = project.columns();
    ASSERT_EQ(project_columns.size(), 1);
    {
        auto&& column = project_columns[0];
        EXPECT_EQ(column.value(), vref(scan_columns[1].destination())); // v
    }

    EXPECT_EQ(relation_columns[0].variable(), project_columns[0].variable());
}

TEST_F(analyze_query_expression_select_test, where) {
    auto table = install_table("testing");
    trelation::graph_type graph {};

    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::query {
                    {
                            ast::query::select_column { vref(id("v")) },
                    },
                    {
                            ast::table::table_reference {
                                    id("testing"),
                            }
                    },
                    {
                            ast::scalar::comparison_predicate {
                                    vref(id("k")),
                                    ast::scalar::comparison_operator::equals,
                                    literal(number("1")),
                            }
                    },
            },
            {},
            {});
    ASSERT_TRUE(r);
    EXPECT_EQ(graph.size(), 3);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();
    EXPECT_EQ(relation.identifier(), "");

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 1);

    // scan - filter - project -
    auto&& project = downcast<trelation::project>(r.output().owner());
    auto&& filter = *find_prev<trelation::filter>(project);
    auto&& scan = *find_prev<trelation::scan>(filter);

    EXPECT_EQ(extract<::yugawara::storage::index>(scan.source()).table(), *table);

    auto&& scan_columns = scan.columns();
    ASSERT_EQ(scan_columns.size(), 4);

    EXPECT_EQ(filter.condition(), (tscalar::compare(
            tscalar::comparison_operator::equal,
            vref(scan_columns[0].destination()),
            immediate(1))));

    auto&& project_columns = project.columns();
    ASSERT_EQ(project_columns.size(), 1);
    {
        auto&& column = project_columns[0];
        EXPECT_EQ(column.value(), vref(scan_columns[1].destination())); // v
    }

    EXPECT_EQ(relation_columns[0].variable(), project_columns[0].variable());
}

} // namespace mizugaki::analyzer::details
