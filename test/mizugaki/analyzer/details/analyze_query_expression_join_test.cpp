#include <mizugaki/analyzer/details/analyze_query_expression.h>

#include <gtest/gtest.h>

#include <takatori/type/primitive.h>

#include <takatori/scalar/binary.h>
#include <takatori/scalar/compare.h>

#include <takatori/relation/scan.h>
#include <takatori/relation/filter.h>
#include <takatori/relation/project.h>
#include <takatori/relation/intermediate/join.h>

#include <yugawara/binding/extract.h>

#include <mizugaki/ast/name/qualified.h>

#include <mizugaki/ast/scalar/binary_expression.h>
#include <mizugaki/ast/scalar/comparison_predicate.h>
#include <mizugaki/ast/scalar/builtin_set_function_invocation.h>

#include <mizugaki/ast/table/table_reference.h>
#include <mizugaki/ast/table/join.h>
#include <mizugaki/ast/table/join_condition.h>
#include <mizugaki/ast/table/join_columns.h>

#include <mizugaki/ast/query/query.h>
#include <mizugaki/ast/query/select_column.h>
#include <mizugaki/ast/query/select_asterisk.h>

#include "test_parent.h"

namespace mizugaki::analyzer::details {

using namespace ::mizugaki::analyzer::testing;

using ::yugawara::binding::extract;

class analyze_query_expression_join_test : public test_parent {
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

TEST_F(analyze_query_expression_join_test, multiple_from) {
    auto table_a = install_table("a");
    auto table_b = install_table("b");
    trelation::graph_type graph {};

    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::query {
                    {
                            ast::query::select_column { vref(ast::name::qualified(id("a"), id("v"))) },
                            ast::query::select_column { vref(ast::name::qualified(id("b"), id("w"))) },
                    },
                    {
                            ast::table::table_reference { id("a") },
                            ast::table::table_reference { id("b") },
                    },
            },
            {},
            {});
    ASSERT_TRUE(r);
    expect_no_error();

    EXPECT_EQ(graph.size(), 4);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();
    EXPECT_EQ(relation.identifier(), "");

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 2);

    // scan*2 - join - project -
    auto&& project = downcast<trelation::project>(r.output().owner());
    auto&& join = *find_prev<trelation::intermediate::join>(project);
    auto&& left = downcast<trelation::scan>(join.left().opposite()->owner());
    auto&& right = downcast<trelation::scan>(join.right().opposite()->owner());

    EXPECT_EQ(extract<::yugawara::storage::index>(left.source()).table(), *table_a);
    EXPECT_EQ(extract<::yugawara::storage::index>(right.source()).table(), *table_b);

    auto&& left_columns = left.columns();
    ASSERT_EQ(left_columns.size(), 4);

    auto&& right_columns = right.columns();
    ASSERT_EQ(right_columns.size(), 4);

    EXPECT_EQ(join.operator_kind(), trelation::join_kind::inner);
    EXPECT_EQ(join.lower(), trelation::intermediate::join::endpoint());
    EXPECT_EQ(join.upper(), trelation::intermediate::join::endpoint());
    EXPECT_FALSE(join.condition());

    auto&& project_columns = project.columns();
    ASSERT_EQ(project_columns.size(), 2);
    {
        auto&& column = project_columns[0];
        EXPECT_EQ(column.value(), vref(left_columns[1].destination())); // a.v
    }
    {
        auto&& column = project_columns[1];
        EXPECT_EQ(column.value(), vref(right_columns[2].destination())); // b.w
    }

    EXPECT_EQ(relation_columns[0].variable(), project_columns[0].variable());
    EXPECT_EQ(relation_columns[1].variable(), project_columns[1].variable());
}

TEST_F(analyze_query_expression_join_test, join_cross) {
    auto table_a = install_table("a");
    auto table_b = install_table("b");
    trelation::graph_type graph {};

    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::query {
                    {
                            ast::query::select_column { vref(ast::name::qualified(id("a"), id("v"))) },
                            ast::query::select_column { vref(ast::name::qualified(id("b"), id("w"))) },
                    },
                    {
                            ast::table::join {
                                    ast::table::table_reference { id("a") },
                                    ast::table::join_type::cross,
                                    ast::table::table_reference { id("b") },
                            }
                    },
            },
            {},
            {});
    ASSERT_TRUE(r);
    expect_no_error();

    EXPECT_EQ(graph.size(), 4);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();
    EXPECT_EQ(relation.identifier(), "");

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 2);

    // scan*2 - join - project -
    auto&& project = downcast<trelation::project>(r.output().owner());
    auto&& join = *find_prev<trelation::intermediate::join>(project);
    auto&& left = downcast<trelation::scan>(join.left().opposite()->owner());
    auto&& right = downcast<trelation::scan>(join.right().opposite()->owner());

    EXPECT_EQ(extract<::yugawara::storage::index>(left.source()).table(), *table_a);
    EXPECT_EQ(extract<::yugawara::storage::index>(right.source()).table(), *table_b);

    auto&& left_columns = left.columns();
    ASSERT_EQ(left_columns.size(), 4);

    auto&& right_columns = right.columns();
    ASSERT_EQ(right_columns.size(), 4);

    EXPECT_EQ(join.operator_kind(), trelation::join_kind::inner);
    EXPECT_EQ(join.lower(), trelation::intermediate::join::endpoint());
    EXPECT_EQ(join.upper(), trelation::intermediate::join::endpoint());
    EXPECT_FALSE(join.condition());

    auto&& project_columns = project.columns();
    ASSERT_EQ(project_columns.size(), 2);
    {
        auto&& column = project_columns[0];
        EXPECT_EQ(column.value(), vref(left_columns[1].destination())); // a.v
    }
    {
        auto&& column = project_columns[1];
        EXPECT_EQ(column.value(), vref(right_columns[2].destination())); // b.w
    }

    EXPECT_EQ(relation_columns[0].variable(), project_columns[0].variable());
    EXPECT_EQ(relation_columns[1].variable(), project_columns[1].variable());
}

TEST_F(analyze_query_expression_join_test, join_inner) {
    auto table_a = install_table("a");
    auto table_b = install_table("b");
    trelation::graph_type graph {};

    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::query {
                    {
                            ast::query::select_column { vref(ast::name::qualified(id("a"), id("v"))) },
                            ast::query::select_column { vref(ast::name::qualified(id("b"), id("w"))) },
                    },
                    {
                            ast::table::join {
                                    ast::table::table_reference { id("a") },
                                    ast::table::join_type::inner,
                                    ast::table::table_reference { id("b") },
                                    ast::table::join_condition {
                                            ast::scalar::comparison_predicate {
                                                    vref(ast::name::qualified(id("a"), id("k"))),
                                                    ast::scalar::comparison_operator::equals,
                                                    vref(ast::name::qualified(id("b"), id("k"))),
                                            }
                                    }
                            }
                    },
            },
            {},
            {});
    ASSERT_TRUE(r);
    expect_no_error();

    EXPECT_EQ(graph.size(), 4);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();
    EXPECT_EQ(relation.identifier(), "");

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 2);

    // scan*2 - join - project -
    auto&& project = downcast<trelation::project>(r.output().owner());
    auto&& join = *find_prev<trelation::intermediate::join>(project);
    auto&& left = downcast<trelation::scan>(join.left().opposite()->owner());
    auto&& right = downcast<trelation::scan>(join.right().opposite()->owner());

    EXPECT_EQ(extract<::yugawara::storage::index>(left.source()).table(), *table_a);
    EXPECT_EQ(extract<::yugawara::storage::index>(right.source()).table(), *table_b);

    auto&& left_columns = left.columns();
    ASSERT_EQ(left_columns.size(), 4);

    auto&& right_columns = right.columns();
    ASSERT_EQ(right_columns.size(), 4);

    EXPECT_EQ(join.condition(), (tscalar::compare(
            tscalar::comparison_operator::equal,
            vref(left_columns[0].destination()),
            vref(right_columns[0].destination()))));

    auto&& project_columns = project.columns();
    ASSERT_EQ(project_columns.size(), 2);
    {
        auto&& column = project_columns[0];
        EXPECT_EQ(column.value(), vref(left_columns[1].destination())); // a.v
    }
    {
        auto&& column = project_columns[1];
        EXPECT_EQ(column.value(), vref(right_columns[2].destination())); // b.w
    }

    EXPECT_EQ(relation_columns[0].variable(), project_columns[0].variable());
    EXPECT_EQ(relation_columns[1].variable(), project_columns[1].variable());
}

TEST_F(analyze_query_expression_join_test, join_left) {
    auto table_a = install_table("a");
    auto table_b = install_table("b");
    trelation::graph_type graph {};

    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::query {
                    {
                            ast::query::select_column { vref(ast::name::qualified(id("a"), id("v"))) },
                            ast::query::select_column { vref(ast::name::qualified(id("b"), id("w"))) },
                    },
                    {
                            ast::table::join {
                                    ast::table::table_reference { id("a") },
                                    ast::table::join_type::left_outer,
                                    ast::table::table_reference { id("b") },
                                    ast::table::join_condition {
                                            ast::scalar::comparison_predicate {
                                                    vref(ast::name::qualified(id("a"), id("k"))),
                                                    ast::scalar::comparison_operator::equals,
                                                    vref(ast::name::qualified(id("b"), id("k"))),
                                            }
                                    }
                            }
                    },
            },
            {},
            {});
    ASSERT_TRUE(r);
    expect_no_error();

    EXPECT_EQ(graph.size(), 4);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();
    EXPECT_EQ(relation.identifier(), "");

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 2);

    // scan*2 - join - project -
    auto&& project = downcast<trelation::project>(r.output().owner());
    auto&& join = *find_prev<trelation::intermediate::join>(project);
    auto&& left = downcast<trelation::scan>(join.left().opposite()->owner());
    auto&& right = downcast<trelation::scan>(join.right().opposite()->owner());

    EXPECT_EQ(extract<::yugawara::storage::index>(left.source()).table(), *table_a);
    EXPECT_EQ(extract<::yugawara::storage::index>(right.source()).table(), *table_b);

    auto&& left_columns = left.columns();
    ASSERT_EQ(left_columns.size(), 4);

    auto&& right_columns = right.columns();
    ASSERT_EQ(right_columns.size(), 4);

    EXPECT_EQ(join.operator_kind(), trelation::join_kind::left_outer);
    EXPECT_EQ(join.lower(), trelation::intermediate::join::endpoint());
    EXPECT_EQ(join.upper(), trelation::intermediate::join::endpoint());
    EXPECT_EQ(join.condition(), (tscalar::compare(
            tscalar::comparison_operator::equal,
            vref(left_columns[0].destination()),
            vref(right_columns[0].destination()))));

    auto&& project_columns = project.columns();
    ASSERT_EQ(project_columns.size(), 2);
    {
        auto&& column = project_columns[0];
        EXPECT_EQ(column.value(), vref(left_columns[1].destination())); // a.v
    }
    {
        auto&& column = project_columns[1];
        EXPECT_EQ(column.value(), vref(right_columns[2].destination())); // b.w
    }

    EXPECT_EQ(relation_columns[0].variable(), project_columns[0].variable());
    EXPECT_EQ(relation_columns[1].variable(), project_columns[1].variable());
}

TEST_F(analyze_query_expression_join_test, join_right) {
    auto table_a = install_table("a");
    auto table_b = install_table("b");
    trelation::graph_type graph {};

    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::query {
                    {
                            ast::query::select_column { vref(ast::name::qualified(id("a"), id("v"))) },
                            ast::query::select_column { vref(ast::name::qualified(id("b"), id("w"))) },
                    },
                    {
                            ast::table::join {
                                    ast::table::table_reference { id("a") },
                                    ast::table::join_type::right_outer,
                                    ast::table::table_reference { id("b") },
                                    ast::table::join_condition {
                                            ast::scalar::comparison_predicate {
                                                    vref(ast::name::qualified(id("a"), id("k"))),
                                                    ast::scalar::comparison_operator::equals,
                                                    vref(ast::name::qualified(id("b"), id("k"))),
                                            }
                                    }
                            }
                    },
            },
            {},
            {});
    ASSERT_TRUE(r);
    expect_no_error();

    EXPECT_EQ(graph.size(), 4);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();
    EXPECT_EQ(relation.identifier(), "");

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 2);

    // scan*2 - join - project -
    auto&& project = downcast<trelation::project>(r.output().owner());
    auto&& join = *find_prev<trelation::intermediate::join>(project);
    auto&& left = downcast<trelation::scan>(join.left().opposite()->owner());
    auto&& right = downcast<trelation::scan>(join.right().opposite()->owner());

    EXPECT_EQ(extract<::yugawara::storage::index>(left.source()).table(), *table_b);
    EXPECT_EQ(extract<::yugawara::storage::index>(right.source()).table(), *table_a);

    auto&& left_columns = left.columns();
    ASSERT_EQ(left_columns.size(), 4);

    auto&& right_columns = right.columns();
    ASSERT_EQ(right_columns.size(), 4);

    EXPECT_EQ(join.operator_kind(), trelation::join_kind::left_outer);
    EXPECT_EQ(join.lower(), trelation::intermediate::join::endpoint());
    EXPECT_EQ(join.upper(), trelation::intermediate::join::endpoint());
    EXPECT_EQ(join.condition(), (tscalar::compare(
            tscalar::comparison_operator::equal,
            vref(right_columns[0].destination()),
            vref(left_columns[0].destination()))));

    auto&& project_columns = project.columns();
    ASSERT_EQ(project_columns.size(), 2);
    {
        auto&& column = project_columns[0];
        EXPECT_EQ(column.value(), vref(right_columns[1].destination())); // a.v
    }
    {
        auto&& column = project_columns[1];
        EXPECT_EQ(column.value(), vref(left_columns[2].destination())); // b.w
    }

    EXPECT_EQ(relation_columns[0].variable(), project_columns[0].variable());
    EXPECT_EQ(relation_columns[1].variable(), project_columns[1].variable());
}

TEST_F(analyze_query_expression_join_test, join_full) {
    auto table_a = install_table("a");
    auto table_b = install_table("b");
    trelation::graph_type graph {};

    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::query {
                    {
                            ast::query::select_column { vref(ast::name::qualified(id("a"), id("v"))) },
                            ast::query::select_column { vref(ast::name::qualified(id("b"), id("w"))) },
                    },
                    {
                            ast::table::join {
                                    ast::table::table_reference { id("a") },
                                    ast::table::join_type::full_outer,
                                    ast::table::table_reference { id("b") },
                                    ast::table::join_condition {
                                            ast::scalar::comparison_predicate {
                                                    vref(ast::name::qualified(id("a"), id("k"))),
                                                    ast::scalar::comparison_operator::equals,
                                                    vref(ast::name::qualified(id("b"), id("k"))),
                                            }
                                    }
                            }
                    },
            },
            {},
            {});
    ASSERT_TRUE(r);
    expect_no_error();

    EXPECT_EQ(graph.size(), 4);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();
    EXPECT_EQ(relation.identifier(), "");

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 2);

    // scan*2 - join - project -
    auto&& project = downcast<trelation::project>(r.output().owner());
    auto&& join = *find_prev<trelation::intermediate::join>(project);
    auto&& left = downcast<trelation::scan>(join.left().opposite()->owner());
    auto&& right = downcast<trelation::scan>(join.right().opposite()->owner());

    EXPECT_EQ(extract<::yugawara::storage::index>(left.source()).table(), *table_a);
    EXPECT_EQ(extract<::yugawara::storage::index>(right.source()).table(), *table_b);

    auto&& left_columns = left.columns();
    ASSERT_EQ(left_columns.size(), 4);

    auto&& right_columns = right.columns();
    ASSERT_EQ(right_columns.size(), 4);

    EXPECT_EQ(join.operator_kind(), trelation::join_kind::full_outer);
    EXPECT_EQ(join.lower(), trelation::intermediate::join::endpoint());
    EXPECT_EQ(join.upper(), trelation::intermediate::join::endpoint());
    EXPECT_EQ(join.condition(), (tscalar::compare(
            tscalar::comparison_operator::equal,
            vref(left_columns[0].destination()),
            vref(right_columns[0].destination()))));

    auto&& project_columns = project.columns();
    ASSERT_EQ(project_columns.size(), 2);
    {
        auto&& column = project_columns[0];
        EXPECT_EQ(column.value(), vref(left_columns[1].destination())); // a.v
    }
    {
        auto&& column = project_columns[1];
        EXPECT_EQ(column.value(), vref(right_columns[2].destination())); // b.w
    }

    EXPECT_EQ(relation_columns[0].variable(), project_columns[0].variable());
    EXPECT_EQ(relation_columns[1].variable(), project_columns[1].variable());
}

TEST_F(analyze_query_expression_join_test, self_join) {
    auto table = install_table("t");
    trelation::graph_type graph {};

    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::query {
                    {
                            ast::query::select_asterisk {},
                    },
                    {
                            ast::table::join {
                                    ast::table::table_reference {
                                            id("t"),
                                            id("t1"),
                                    },
                                    ast::table::join_type::inner,
                                    ast::table::table_reference {
                                            id("t"),
                                            id("t2"),
                                    },
                                    ast::table::join_condition{
                                            ast::scalar::comparison_predicate {
                                                    vref(ast::name::qualified(id("t1"), id("k"))),
                                                    ast::scalar::comparison_operator::equals,
                                                    vref(ast::name::qualified(id("t2"), id("k"))),
                                            }
                                    },
                            }
                    },
            },
            {},
            {});
    ASSERT_TRUE(r);
    expect_no_error();

    EXPECT_EQ(graph.size(), 4);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();
    EXPECT_EQ(relation.identifier(), "");

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 8);

    // scan*2 - join - project -
    auto&& project = downcast<trelation::project>(r.output().owner());
    auto&& join = *find_prev<trelation::intermediate::join>(project);
    auto&& left = downcast<trelation::scan>(join.left().opposite()->owner());
    auto&& right = downcast<trelation::scan>(join.right().opposite()->owner());

    EXPECT_EQ(extract<::yugawara::storage::index>(left.source()).table(), *table);
    EXPECT_EQ(extract<::yugawara::storage::index>(right.source()).table(), *table);

    auto&& left_columns = left.columns();
    ASSERT_EQ(left_columns.size(), 4);

    auto&& right_columns = right.columns();
    ASSERT_EQ(right_columns.size(), 4);

    EXPECT_EQ(join.operator_kind(), trelation::join_kind::inner);
    EXPECT_EQ(join.lower(), trelation::intermediate::join::endpoint());
    EXPECT_EQ(join.upper(), trelation::intermediate::join::endpoint());
    EXPECT_EQ(join.condition(), (tscalar::compare {
            tscalar::comparison_operator::equal,
            vref(left_columns[0].destination()),
            vref(right_columns[0].destination()),
    }));

    auto&& project_columns = project.columns();
    ASSERT_EQ(project_columns.size(), 0);

    EXPECT_EQ(relation_columns[0].variable(), left_columns[0].destination());
    EXPECT_EQ(relation_columns[1].variable(), left_columns[1].destination());
    EXPECT_EQ(relation_columns[2].variable(), left_columns[2].destination());
    EXPECT_EQ(relation_columns[3].variable(), left_columns[3].destination());
    EXPECT_EQ(relation_columns[4].variable(), right_columns[0].destination());
    EXPECT_EQ(relation_columns[5].variable(), right_columns[1].destination());
    EXPECT_EQ(relation_columns[6].variable(), right_columns[2].destination());
    EXPECT_EQ(relation_columns[7].variable(), right_columns[3].destination());
}

TEST_F(analyze_query_expression_join_test, join_columns) {
    auto table_a = install_table("a");
    auto table_b = install_table("b");
    trelation::graph_type graph {};

    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::query {
                    {
                            ast::query::select_column { vref(ast::name::qualified(id("a"), id("v"))) },
                            ast::query::select_column { vref(ast::name::qualified(id("b"), id("w"))) },
                    },
                    {
                            ast::table::join {
                                    ast::table::table_reference { id("a") },
                                    ast::table::join_type::inner,
                                    ast::table::table_reference { id("b") },
                                    ast::table::join_columns {
                                            id("k")
                                    }
                            }
                    },
            },
            {},
            {});
    ASSERT_TRUE(r);
    expect_no_error();

    EXPECT_EQ(graph.size(), 4);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();
    EXPECT_EQ(relation.identifier(), "");

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 2);

    // scan*2 - join - project -
    auto&& project = downcast<trelation::project>(r.output().owner());
    auto&& join = *find_prev<trelation::intermediate::join>(project);
    auto&& left = downcast<trelation::scan>(join.left().opposite()->owner());
    auto&& right = downcast<trelation::scan>(join.right().opposite()->owner());

    EXPECT_EQ(extract<::yugawara::storage::index>(left.source()).table(), *table_a);
    EXPECT_EQ(extract<::yugawara::storage::index>(right.source()).table(), *table_b);

    auto&& left_columns = left.columns();
    ASSERT_EQ(left_columns.size(), 4);

    auto&& right_columns = right.columns();
    ASSERT_EQ(right_columns.size(), 4);

    EXPECT_EQ(join.condition(), (tscalar::compare(
            tscalar::comparison_operator::equal,
            vref(left_columns[0].destination()),
            vref(right_columns[0].destination()))));

    auto&& project_columns = project.columns();
    ASSERT_EQ(project_columns.size(), 2);
    {
        auto&& column = project_columns[0];
        EXPECT_EQ(column.value(), vref(left_columns[1].destination())); // a.v
    }
    {
        auto&& column = project_columns[1];
        EXPECT_EQ(column.value(), vref(right_columns[2].destination())); // b.w
    }

    EXPECT_EQ(relation_columns[0].variable(), project_columns[0].variable());
    EXPECT_EQ(relation_columns[1].variable(), project_columns[1].variable());
}

TEST_F(analyze_query_expression_join_test, join_columns_multiple) {
    auto table_a = install_table("a");
    auto table_b = install_table("b");
    trelation::graph_type graph {};

    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::query {
                    {
                            ast::query::select_column { vref(ast::name::qualified(id("a"), id("k"))) },
                            ast::query::select_column { vref(ast::name::qualified(id("b"), id("k"))) },
                    },
                    {
                            ast::table::join {
                                    ast::table::table_reference { id("a") },
                                    ast::table::join_type::inner,
                                    ast::table::table_reference { id("b") },
                                    ast::table::join_columns {
                                            id("v"),
                                            id("w"),
                                            id("x"),
                                    }
                            }
                    },
            },
            {},
            {});
    ASSERT_TRUE(r);
    expect_no_error();

    EXPECT_EQ(graph.size(), 4);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();
    EXPECT_EQ(relation.identifier(), "");

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 2);

    // scan*2 - join - project -
    auto&& project = downcast<trelation::project>(r.output().owner());
    auto&& join = *find_prev<trelation::intermediate::join>(project);
    auto&& left = downcast<trelation::scan>(join.left().opposite()->owner());
    auto&& right = downcast<trelation::scan>(join.right().opposite()->owner());

    EXPECT_EQ(extract<::yugawara::storage::index>(left.source()).table(), *table_a);
    EXPECT_EQ(extract<::yugawara::storage::index>(right.source()).table(), *table_b);

    auto&& left_columns = left.columns();
    ASSERT_EQ(left_columns.size(), 4);

    auto&& right_columns = right.columns();
    ASSERT_EQ(right_columns.size(), 4);

    EXPECT_EQ(join.condition(), (tscalar::binary {
            tscalar::binary_operator::conditional_and,
            tscalar::binary {
                    tscalar::binary_operator::conditional_and,
                    tscalar::compare {
                            tscalar::comparison_operator::equal,
                            vref(left_columns[1].destination()), // a.v
                            vref(right_columns[1].destination()), // b.v
                    },
                    tscalar::compare {
                            tscalar::comparison_operator::equal,
                            vref(left_columns[2].destination()), // a.w
                            vref(right_columns[2].destination()), // b/w
                    },
            },
            tscalar::compare {
                    tscalar::comparison_operator::equal,
                    vref(left_columns[3].destination()), // a.x
                    vref(right_columns[3].destination()), // b.x
            },
    }));

    auto&& project_columns = project.columns();
    ASSERT_EQ(project_columns.size(), 2);
    EXPECT_EQ(project_columns[0].value(), vref(left_columns[0].destination())); // a.k
    EXPECT_EQ(project_columns[1].value(), vref(right_columns[0].destination())); // b.k

    EXPECT_EQ(relation_columns[0].variable(), project_columns[0].variable());
    EXPECT_EQ(relation_columns[1].variable(), project_columns[1].variable());
}

TEST_F(analyze_query_expression_join_test, join_condition_invalid_expression) {
    auto table_a = install_table("a");
    auto table_b = install_table("b");
    invalid(ast::query::query {
            {
                    ast::query::select_column { vref(ast::name::qualified(id("a"), id("v"))) },
                    ast::query::select_column { vref(ast::name::qualified(id("b"), id("w"))) },
            },
            {
                    ast::table::join {
                            ast::table::table_reference { id("a") },
                            ast::table::join_type::inner,
                            ast::table::table_reference { id("b") },
                            ast::table::join_condition {
                                    ast::scalar::comparison_predicate {
                                            vref(id("k")),
                                            ast::scalar::comparison_operator::equals,
                                            vref(id("k")),
                                    }
                            }
                    }
            },
    });
}

TEST_F(analyze_query_expression_join_test, join_condition_invalid_aggregated) {
    auto table_a = install_table("a");
    auto table_b = install_table("b");
    auto count = set_functions_->add(::yugawara::aggregate::declaration {
            ::yugawara::aggregate::declaration::minimum_builtin_function_id + 1,
            "count",
            ttype::int8 {},
            {},
            true,
    });
    invalid(sql_analyzer_code::unsupported_feature, ast::query::query {
            {
                    ast::query::select_column { vref(ast::name::qualified(id("a"), id("v"))) },
                    ast::query::select_column { vref(ast::name::qualified(id("b"), id("w"))) },
            },
            {
                    ast::table::join {
                            ast::table::table_reference { id("a") },
                            ast::table::join_type::inner,
                            ast::table::table_reference { id("b") },
                            ast::table::join_condition {
                                    ast::scalar::comparison_predicate {
                                            ast::scalar::builtin_set_function_invocation {
                                                    ast::scalar::builtin_set_function_kind::count,
                                                    {},
                                                    {},
                                            },
                                            ast::scalar::comparison_operator::equals,
                                            ast::scalar::binary_expression {
                                                    vref(ast::name::qualified(id("a"), id("k"))),
                                                    ast::scalar::binary_operator::plus,
                                                    vref(ast::name::qualified(id("b"), id("k"))),
                                            }
                                    }
                            }
                    }
            },
    });
}

TEST_F(analyze_query_expression_join_test, join_columns_missing) {
    auto table_a = install_table("a");
    auto table_b = install_table("b");
    invalid(sql_analyzer_code::column_not_found, ast::query::query {
            {
                    ast::query::select_column { vref(ast::name::qualified(id("a"), id("v"))) },
                    ast::query::select_column { vref(ast::name::qualified(id("b"), id("w"))) },
            },
            {
                    ast::table::join {
                            ast::table::table_reference { id("a") },
                            ast::table::join_type::inner,
                            ast::table::table_reference { id("b") },
                            ast::table::join_columns {
                                    id("MISSING")
                            }
                    }
            },
    });
}

TEST_F(analyze_query_expression_join_test, qualified_asterisk_left) {
    auto table_a = install_table("a");
    auto table_b = install_table("b");
    trelation::graph_type graph {};

    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::query {
                    {
                            ast::query::select_asterisk { vref(id("a")) },
                    },
                    {
                            ast::table::table_reference { id("a") },
                            ast::table::table_reference { id("b") },
                    },
            },
            {},
            {});
    ASSERT_TRUE(r);
    expect_no_error();

    EXPECT_EQ(graph.size(), 4);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();
    EXPECT_EQ(relation.identifier(), "");

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 4);

    // scan*2 - join - project -
    auto&& project = downcast<trelation::project>(r.output().owner());
    auto&& join = *find_prev<trelation::intermediate::join>(project);
    auto&& left = downcast<trelation::scan>(join.left().opposite()->owner());
    auto&& right = downcast<trelation::scan>(join.right().opposite()->owner());

    auto&& left_columns = left.columns();
    ASSERT_EQ(left_columns.size(), 4);

    auto&& right_columns = right.columns();
    ASSERT_EQ(right_columns.size(), 4);

    auto&& project_columns = project.columns();
    ASSERT_EQ(project_columns.size(), 0);

    EXPECT_EQ(relation_columns[0].variable(), left_columns[0].destination());
    EXPECT_EQ(relation_columns[1].variable(), left_columns[1].destination());
    EXPECT_EQ(relation_columns[2].variable(), left_columns[2].destination());
    EXPECT_EQ(relation_columns[3].variable(), left_columns[3].destination());
}

TEST_F(analyze_query_expression_join_test, qualified_asterisk_right) {
    auto table_a = install_table("a");
    auto table_b = install_table("b");
    trelation::graph_type graph {};

    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::query {
                    {
                            ast::query::select_asterisk { vref(id("public", "b")) },
                    },
                    {
                            ast::table::table_reference { id("a") },
                            ast::table::table_reference { id("b") },
                    },
            },
            {},
            {});
    ASSERT_TRUE(r);
    expect_no_error();

    EXPECT_EQ(graph.size(), 4);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();
    EXPECT_EQ(relation.identifier(), "");

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 4);

    // scan*2 - join - project -
    auto&& project = downcast<trelation::project>(r.output().owner());
    auto&& join = *find_prev<trelation::intermediate::join>(project);
    auto&& left = downcast<trelation::scan>(join.left().opposite()->owner());
    auto&& right = downcast<trelation::scan>(join.right().opposite()->owner());

    auto&& left_columns = left.columns();
    ASSERT_EQ(left_columns.size(), 4);

    auto&& right_columns = right.columns();
    ASSERT_EQ(right_columns.size(), 4);

    auto&& project_columns = project.columns();
    ASSERT_EQ(project_columns.size(), 0);

    EXPECT_EQ(relation_columns[0].variable(), right_columns[0].destination());
    EXPECT_EQ(relation_columns[1].variable(), right_columns[1].destination());
    EXPECT_EQ(relation_columns[2].variable(), right_columns[2].destination());
    EXPECT_EQ(relation_columns[3].variable(), right_columns[3].destination());
}

} // namespace mizugaki::analyzer::details
