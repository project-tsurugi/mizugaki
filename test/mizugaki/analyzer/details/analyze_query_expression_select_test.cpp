#include <mizugaki/analyzer/details/analyze_query_expression.h>

#include <gtest/gtest.h>

#include <takatori/type/primitive.h>

#include <takatori/scalar/compare.h>

#include <takatori/relation/scan.h>
#include <takatori/relation/filter.h>
#include <takatori/relation/project.h>
#include <takatori/relation/values.h>
#include <takatori/relation/intermediate/distinct.h>
#include <takatori/relation/intermediate/limit.h>

#include <yugawara/binding/extract.h>

#include <mizugaki/ast/name/qualified.h>

#include <mizugaki/ast/scalar/comparison_predicate.h>

#include <mizugaki/ast/table/table_reference.h>

#include <mizugaki/ast/query/query.h>
#include <mizugaki/ast/query/select_asterisk.h>
#include <mizugaki/ast/query/select_column.h>
#include <mizugaki/ast/query/grouping_column.h>

#include "test_parent.h"

namespace mizugaki::analyzer::details {

using namespace ::mizugaki::analyzer::testing;

using ::yugawara::binding::extract;

class analyze_query_expression_select_test : public test_parent {
protected:
    void invalid(ast::query::expression const& expression) {
        trelation::graph_type graph {};
        auto r = analyze_query_expression(
                context(),
                graph,
                expression,
                {},
                {});
        EXPECT_FALSE(r) << diagnostics();
        EXPECT_NE(count_error(), 0);
    }
};

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
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(graph.size(), 2);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();
    EXPECT_EQ(relation.identifier(), "");

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 1);
    {
        auto&& column = relation_columns[0];
        EXPECT_EQ(column.identifier(), "v");
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
    expect_no_error();

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
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

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
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

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
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

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

TEST_F(analyze_query_expression_select_test, without_tables) {
    trelation::graph_type graph {};

    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::query {
                    {
                            ast::query::select_column {
                                    literal(number("1")),
                                    id("r"),
                            }
                    },
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(graph.size(), 2);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();
    EXPECT_EQ(relation.identifier(), "");

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 1);
    {
        auto&& column = relation_columns[0];
        EXPECT_EQ(column.identifier(), "r");
        EXPECT_TRUE(column.exported());
    }

    // values - project -
    auto&& project = downcast<trelation::project>(r.output().owner());
    auto&& values = *find_prev<trelation::values>(project);

    ASSERT_EQ(values.columns().size(), 0);
    ASSERT_EQ(values.rows().size(), 1);
    ASSERT_EQ(values.rows()[0].elements().size(), 0);

    auto&& project_columns = project.columns();
    ASSERT_EQ(project_columns.size(), 1);
    {
        auto&& column = project_columns[0];
        EXPECT_EQ(column.value(), immediate(1));
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
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

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

TEST_F(analyze_query_expression_select_test, distinct) {
    auto table = install_table("testing");
    trelation::graph_type graph {};

    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::query {
                    ast::query::set_quantifier::distinct,
                    {
                            ast::query::select_column { vref(id("v")) },
                    },
                    {
                            ast::table::table_reference {
                                    id("testing"),
                            }
                    },
                    {},
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    // select - project - distinct -
    EXPECT_EQ(graph.size(), 3);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();
    EXPECT_EQ(relation.identifier(), "");

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 1);

    // scan - project - distinct -
    auto&& distinct = downcast<trelation::intermediate::distinct>(r.output().owner());
    auto&& project = *find_prev<trelation::project>(distinct);
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

    auto&& distinct_columns = distinct.group_keys();
    ASSERT_EQ(distinct_columns.size(), 1);
    EXPECT_EQ(distinct_columns[0], project_columns[0].variable());
}

TEST_F(analyze_query_expression_select_test, distinct_multiple_columns) {
    auto table = install_table("testing");
    trelation::graph_type graph {};

    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::query {
                    ast::query::set_quantifier::distinct,
                    {
                            ast::query::select_column { vref(id("v")) },
                            ast::query::select_column { vref(id("w")) },
                            ast::query::select_column { vref(id("x")) },
                    },
                    {
                            ast::table::table_reference {
                                    id("testing"),
                            }
                    },
                    {},
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    // select - project - distinct -
    EXPECT_EQ(graph.size(), 3);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();
    EXPECT_EQ(relation.identifier(), "");

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 3);

    // scan - project - distinct -
    auto&& distinct = downcast<trelation::intermediate::distinct>(r.output().owner());
    auto&& project = *find_prev<trelation::project>(distinct);
    auto&& scan = *find_prev<trelation::scan>(project);

    EXPECT_EQ(extract<::yugawara::storage::index>(scan.source()).table(), *table);

    auto&& scan_columns = scan.columns();
    ASSERT_EQ(scan_columns.size(), 4);

    auto&& project_columns = project.columns();
    ASSERT_EQ(project_columns.size(), 3);
    EXPECT_EQ(project_columns[0].value(), vref(scan_columns[1].destination())); // v
    EXPECT_EQ(project_columns[1].value(), vref(scan_columns[2].destination())); // v
    EXPECT_EQ(project_columns[2].value(), vref(scan_columns[3].destination())); // v

    EXPECT_EQ(relation_columns[0].variable(), project_columns[0].variable());

    auto&& distinct_columns = distinct.group_keys();
    ASSERT_EQ(distinct_columns.size(), 3);
    EXPECT_EQ(distinct_columns[0], project_columns[0].variable());
    EXPECT_EQ(distinct_columns[1], project_columns[1].variable());
    EXPECT_EQ(distinct_columns[2], project_columns[2].variable());
}

TEST_F(analyze_query_expression_select_test, order_by) {
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
                    {},
                    {},
                    {},
                    {
                            {
                                    vref(id("w")),
                            },
                    },
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(graph.size(), 4);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();
    EXPECT_EQ(relation.identifier(), "");

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 1);

    // select - project - project - limit -
    auto&& limit = downcast<trelation::intermediate::limit>(r.output().owner());
    auto&& prepare = *find_prev<trelation::project>(limit);
    auto&& project = *find_prev<trelation::project>(prepare);
    auto&& scan = *find_prev<trelation::scan>(project);

    EXPECT_EQ(extract<::yugawara::storage::index>(scan.source()).table(), *table);

    auto&& scan_columns = scan.columns();
    ASSERT_EQ(scan_columns.size(), 4);

    auto&& project_columns = project.columns();
    ASSERT_EQ(project_columns.size(), 1);
    EXPECT_EQ(project_columns[0].value(), vref(scan_columns[1].destination())); // v

    EXPECT_EQ(relation_columns[0].variable(), project_columns[0].variable());

    auto&& prepare_columns = prepare.columns();
    ASSERT_EQ(prepare_columns.size(), 1);
    EXPECT_EQ(prepare_columns[0].value(), vref(scan_columns[2].destination())); // w

    EXPECT_EQ(limit.count(), std::nullopt);
    EXPECT_EQ(limit.group_keys().size(), 0);

    auto&& limit_columns = limit.sort_keys();
    ASSERT_EQ(limit_columns.size(), 1);
    {
        auto&& column = limit_columns[0];
        EXPECT_EQ(column.variable(), prepare_columns[0].variable());
        EXPECT_EQ(column.direction(), trelation::sort_direction::ascendant);
    }
}

TEST_F(analyze_query_expression_select_test, order_by_multiple_columns) {
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
                    {},
                    {},
                    {},
                    {
                            {
                                    vref(id("w")),
                            },
                            {
                                    vref(id("x")),
                                    {},
                                    ast::common::ordering_specification::asc,
                            },
                            {
                                    vref(id("k")),
                                    {},
                                    ast::common::ordering_specification::desc,
                            },
                    },
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(graph.size(), 4);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();
    EXPECT_EQ(relation.identifier(), "");

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 1);

    // select - project - project - limit -
    auto&& limit = downcast<trelation::intermediate::limit>(r.output().owner());
    auto&& prepare = *find_prev<trelation::project>(limit);
    auto&& project = *find_prev<trelation::project>(prepare);
    auto&& scan = *find_prev<trelation::scan>(project);

    EXPECT_EQ(extract<::yugawara::storage::index>(scan.source()).table(), *table);

    auto&& scan_columns = scan.columns();
    ASSERT_EQ(scan_columns.size(), 4);

    auto&& project_columns = project.columns();
    ASSERT_EQ(project_columns.size(), 1);
    EXPECT_EQ(project_columns[0].value(), vref(scan_columns[1].destination())); // v

    EXPECT_EQ(relation_columns[0].variable(), project_columns[0].variable());

    auto&& prepare_columns = prepare.columns();
    ASSERT_EQ(prepare_columns.size(), 3);
    EXPECT_EQ(prepare_columns[0].value(), vref(scan_columns[2].destination())); // w
    EXPECT_EQ(prepare_columns[1].value(), vref(scan_columns[3].destination())); // x
    EXPECT_EQ(prepare_columns[2].value(), vref(scan_columns[0].destination())); // k

    EXPECT_EQ(limit.count(), std::nullopt);
    EXPECT_EQ(limit.group_keys().size(), 0);

    auto&& limit_columns = limit.sort_keys();
    ASSERT_EQ(limit_columns.size(), 3);
    {
        auto&& column = limit_columns[0];
        EXPECT_EQ(column.variable(), prepare_columns[0].variable());
        EXPECT_EQ(column.direction(), trelation::sort_direction::ascendant);
    }
    {
        auto&& column = limit_columns[1];
        EXPECT_EQ(column.variable(), prepare_columns[1].variable());
        EXPECT_EQ(column.direction(), trelation::sort_direction::ascendant);
    }
    {
        auto&& column = limit_columns[2];
        EXPECT_EQ(column.variable(), prepare_columns[2].variable());
        EXPECT_EQ(column.direction(), trelation::sort_direction::descendant);
    }
}

TEST_F(analyze_query_expression_select_test, order_by_expression) {
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
                    {},
                    {},
                    {},
                    {
                            {
                                    literal(number("1")),
                            },
                    },
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(graph.size(), 4);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();
    EXPECT_EQ(relation.identifier(), "");

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 1);

    // select - project - project - limit -
    auto&& limit = downcast<trelation::intermediate::limit>(r.output().owner());
    auto&& prepare = *find_prev<trelation::project>(limit);
    auto&& project = *find_prev<trelation::project>(prepare);
    auto&& scan = *find_prev<trelation::scan>(project);

    EXPECT_EQ(extract<::yugawara::storage::index>(scan.source()).table(), *table);

    auto&& scan_columns = scan.columns();
    ASSERT_EQ(scan_columns.size(), 4);

    auto&& project_columns = project.columns();
    ASSERT_EQ(project_columns.size(), 1);
    EXPECT_EQ(project_columns[0].value(), vref(scan_columns[1].destination())); // v

    EXPECT_EQ(relation_columns[0].variable(), project_columns[0].variable());

    auto&& prepare_columns = prepare.columns();
    ASSERT_EQ(prepare_columns.size(), 1);
    EXPECT_EQ(prepare_columns[0].value(), immediate(1)); // literal: 1

    EXPECT_EQ(limit.count(), std::nullopt);
    EXPECT_EQ(limit.group_keys().size(), 0);

    auto&& limit_columns = limit.sort_keys();
    ASSERT_EQ(limit_columns.size(), 1);
    {
        auto&& column = limit_columns[0];
        EXPECT_EQ(column.variable(), prepare_columns[0].variable());
        EXPECT_EQ(column.direction(), trelation::sort_direction::ascendant);
    }
}

TEST_F(analyze_query_expression_select_test, limit) {
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
                    {},
                    {},
                    {},
                    {},
                    {
                            literal(number("10")),
                    },
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    // select - project - limit -
    EXPECT_EQ(graph.size(), 3);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();
    EXPECT_EQ(relation.identifier(), "");

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 1);

    // scan - project - limit -
    auto&& limit = downcast<trelation::intermediate::limit>(r.output().owner());
    auto&& project = *find_prev<trelation::project>(limit);
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

    EXPECT_EQ(limit.count(), 10);
    EXPECT_EQ(limit.group_keys().size(), 0);
    EXPECT_EQ(limit.sort_keys().size(), 0);
}

TEST_F(analyze_query_expression_select_test, order_by_limit) {
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
                    {},
                    {},
                    {},
                    {
                            {
                                    vref(id("w")),
                            },
                    },
                    {
                            literal(number("5")),
                    },
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(graph.size(), 4);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();
    EXPECT_EQ(relation.identifier(), "");

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 1);

    // select - project - project - limit -
    auto&& limit = downcast<trelation::intermediate::limit>(r.output().owner());
    auto&& prepare = *find_prev<trelation::project>(limit);
    auto&& project = *find_prev<trelation::project>(prepare);
    auto&& scan = *find_prev<trelation::scan>(project);

    EXPECT_EQ(extract<::yugawara::storage::index>(scan.source()).table(), *table);

    auto&& scan_columns = scan.columns();
    ASSERT_EQ(scan_columns.size(), 4);

    auto&& project_columns = project.columns();
    ASSERT_EQ(project_columns.size(), 1);
    EXPECT_EQ(project_columns[0].value(), vref(scan_columns[1].destination())); // v

    EXPECT_EQ(relation_columns[0].variable(), project_columns[0].variable());

    auto&& prepare_columns = prepare.columns();
    ASSERT_EQ(prepare_columns.size(), 1);
    EXPECT_EQ(prepare_columns[0].value(), vref(scan_columns[2].destination())); // w

    EXPECT_EQ(limit.count(), 5);
    EXPECT_EQ(limit.group_keys().size(), 0);

    auto&& limit_columns = limit.sort_keys();
    ASSERT_EQ(limit_columns.size(), 1);
    {
        auto&& column = limit_columns[0];
        EXPECT_EQ(column.variable(), prepare_columns[0].variable());
        EXPECT_EQ(column.direction(), trelation::sort_direction::ascendant);
    }
}

TEST_F(analyze_query_expression_select_test, invalid_select_column) {
    auto table = install_table("testing");
    invalid(ast::query::query {
            {
                    ast::query::select_column { vref(id("MISSING")) },
            },
            {
                    ast::table::table_reference {
                            id("testing"),
                    }
            },
    });
}

TEST_F(analyze_query_expression_select_test, invalid_select_asterisk_table) {
    auto table = install_table("testing");
    invalid(ast::query::query {
            {
                    ast::query::select_asterisk { vref(id("testing")) },
            },
            {
                    ast::table::table_reference {
                            id("testing"),
                    }
            },
    });
}

TEST_F(analyze_query_expression_select_test, invalid_from_table) {
    auto table = install_table("testing");
    invalid(ast::query::query {
            {
                    ast::query::select_asterisk {},
            },
            {
                    ast::table::table_reference {
                            id("MISSING"),
                    }
            },
    });
}

TEST_F(analyze_query_expression_select_test, invalid_where_predicate) {
    auto table = install_table("testing");
    invalid(ast::query::query {
            {
                    ast::query::select_asterisk {},
            },
            {
                    ast::table::table_reference {
                            id("testing"),
                    }
            },
            {
                    ast::scalar::comparison_predicate {
                            vref(id("MISSING")),
                            ast::scalar::comparison_operator::equals,
                            literal(string("''")),
                    }
            },
    });
}

TEST_F(analyze_query_expression_select_test, invalid_group_by_column) {
    auto table = install_table("testing");
    invalid(ast::query::query {
            {
                    ast::query::select_column { vref(id("v")) },
            },
            {
                    ast::table::table_reference {
                            id("testing"),
                    }
            },
            {},
            ast::query::group_by_clause {
                    ast::query::grouping_column { id("v") },
                    ast::query::grouping_column { id("MISSING") },
            },
    });
}

TEST_F(analyze_query_expression_select_test, invalid_having_predicate) {
    auto table = install_table("testing");
    invalid(ast::query::query {
            {
                    ast::query::select_column { vref(id("v")) },
            },
            {
                    ast::table::table_reference {
                            id("testing"),
                    }
            },
            {},
            ast::query::group_by_clause {
                    ast::query::grouping_column { id("v") },
            },
            {
                    ast::scalar::comparison_predicate {
                            vref(id("MISSING")),
                            ast::scalar::comparison_operator::equals,
                            literal(string("''")),
                    }
            },
    });
}

TEST_F(analyze_query_expression_select_test, invalid_order_by_column) {
    auto table = install_table("testing");
    invalid(ast::query::query {
            {
                    ast::query::select_asterisk {},
            },
            {
                    ast::table::table_reference {
                            id("testing"),
                    }
            },
            {},
            {},
            {},
            {
                    {
                            vref(id("MISSING")),
                    },
            },
    });
}

TEST_F(analyze_query_expression_select_test, invalid_limit_expression) {
    auto table = install_table("testing");
    invalid(ast::query::query {
            {
                    ast::query::select_asterisk {},
            },
            {
                    ast::table::table_reference {
                            id("testing"),
                    }
            },
            {},
            {},
            {},
            {},
            {
                    vref(id("MISSING")),
            },
    });
}

TEST_F(analyze_query_expression_select_test, invalid_limit_value_string) {
    auto table = install_table("testing");
    invalid(ast::query::query {
            {
                    ast::query::select_asterisk {},
            },
            {
                    ast::table::table_reference {
                            id("testing"),
                    }
            },
            {},
            {},
            {},
            {},
            {
                    literal(string("''")),
            },
    });
}

TEST_F(analyze_query_expression_select_test, invalid_limit_not_constant) {
    auto table = install_table("testing");
    invalid(ast::query::query {
            {
                    ast::query::select_asterisk {},
            },
            {
                    ast::table::table_reference {
                            id("testing"),
                    }
            },
            {},
            {},
            {},
            {},
            {
                    vref(id("k")),
            },
    });
}

} // namespace mizugaki::analyzer::details
