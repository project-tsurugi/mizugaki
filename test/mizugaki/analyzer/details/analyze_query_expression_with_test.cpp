#include <mizugaki/analyzer/details/analyze_query_expression.h>

#include <gtest/gtest.h>

#include <takatori/type/primitive.h>

#include <takatori/relation/scan.h>
#include <takatori/relation/values.h>
#include <takatori/relation/intermediate/union.h>

#include <yugawara/binding/extract.h>

#include <yugawara/extension/relation/subquery.h>

#include <mizugaki/ast/scalar/value_constructor.h>

#include <mizugaki/ast/query/binary_expression.h>
#include <mizugaki/ast/query/query.h>
#include <mizugaki/ast/query/table_reference.h>
#include <mizugaki/ast/query/table_value_constructor.h>
#include <mizugaki/ast/query/with_expression.h>

#include <mizugaki/ast/table/table_reference.h>

#include "test_parent.h"

namespace mizugaki::analyzer::details {

using namespace ::mizugaki::analyzer::testing;

using ::yugawara::binding::extract;

class analyze_query_expression_with_test : public test_parent {
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

    void invalid(sql_analyzer_code code, ast::query::expression const& expression) {
        invalid(expression);
        EXPECT_TRUE(find_error(code)) << diagnostics();
    }
};

TEST_F(analyze_query_expression_with_test, simple) {
    trelation::graph_type graph {};
    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::with_expression {
                {
                    ast::query::with_element {
                        id("q"),
                        ast::query::table_value_constructor {
                            ast::scalar::value_constructor {
                                literal(number("1")),
                                literal(number("2")),
                                literal(number("3")),
                            },
                        },
                    },
                },
                ast::query::table_reference {
                        id("q"),
                },
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(graph.size(), 1);
    EXPECT_FALSE(r.output().opposite());

    // subquery -
    auto&& subquery = downcast<::yugawara::extension::relation::subquery>(r.output().owner());

    ASSERT_EQ(subquery.mappings().size(), 3);
    auto&& si0 = subquery.mappings()[0].source();
    auto&& si1 = subquery.mappings()[1].source();
    auto&& si2 = subquery.mappings()[2].source();
    auto&& so0 = subquery.mappings()[0].destination();
    auto&& so1 = subquery.mappings()[1].destination();
    auto&& so2 = subquery.mappings()[2].destination();

    auto&& subgraph = subquery.query_graph();
    ASSERT_EQ(subgraph.size(), 1);

    auto&& subquery_output = subquery.find_output_port();
    ASSERT_TRUE(subquery_output);
    ASSERT_FALSE(subquery_output->opposite());
    ASSERT_TRUE(subgraph.contains(subquery_output->owner()));

    // values -
    auto&& values = downcast<trelation::values>(subquery_output->owner());
    auto&& values_columns = values.columns();
    ASSERT_EQ(values_columns.size(), 3);
    EXPECT_EQ(values_columns[0], si0);
    EXPECT_EQ(values_columns[1], si1);
    EXPECT_EQ(values_columns[2], si2);

    auto&& relation = r.relation();
    ASSERT_FALSE(relation.declaration());

    EXPECT_EQ(relation.identifier(), "q");

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 3);
    {
        auto&& column = relation_columns[0];
        EXPECT_EQ(column.identifier(), std::nullopt);
        EXPECT_EQ(column.variable(), so0);
        EXPECT_EQ(column.declaration(), nullptr);
        EXPECT_TRUE(column.exported());
    }
    {
        auto&& column = relation_columns[1];
        EXPECT_EQ(column.identifier(), std::nullopt);
        EXPECT_EQ(column.variable(), so1);
        EXPECT_EQ(column.declaration(), nullptr);
        EXPECT_TRUE(column.exported());
    }
    {
        auto&& column = relation_columns[2];
        EXPECT_EQ(column.identifier(), std::nullopt);
        EXPECT_EQ(column.variable(), so2);
        EXPECT_EQ(column.declaration(), nullptr);
        EXPECT_TRUE(column.exported());
    }
}

TEST_F(analyze_query_expression_with_test, column_names) {
    trelation::graph_type graph {};
    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::with_expression {
                {
                    ast::query::with_element {
                        id("q"),
                        {
                                id("a"),
                                id("b"),
                                id("c"),
                        },
                        ast::query::table_value_constructor {
                            ast::scalar::value_constructor {
                                literal(number("1")),
                                literal(number("2")),
                                literal(number("3")),
                            },
                        },
                    },
                },
                ast::query::table_reference {
                        id("q"),
                },
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(graph.size(), 1);
    EXPECT_FALSE(r.output().opposite());

    // subquery -
    auto&& subquery = downcast<::yugawara::extension::relation::subquery>(r.output().owner());

    ASSERT_EQ(subquery.mappings().size(), 3);
    auto&& si0 = subquery.mappings()[0].source();
    auto&& si1 = subquery.mappings()[1].source();
    auto&& si2 = subquery.mappings()[2].source();
    auto&& so0 = subquery.mappings()[0].destination();
    auto&& so1 = subquery.mappings()[1].destination();
    auto&& so2 = subquery.mappings()[2].destination();

    auto&& subgraph = subquery.query_graph();
    ASSERT_EQ(subgraph.size(), 1);

    auto&& subquery_output = subquery.find_output_port();
    ASSERT_TRUE(subquery_output);
    ASSERT_FALSE(subquery_output->opposite());
    ASSERT_TRUE(subgraph.contains(subquery_output->owner()));

    // values -
    auto&& values = downcast<trelation::values>(subquery_output->owner());
    auto&& values_columns = values.columns();
    ASSERT_EQ(values_columns.size(), 3);
    EXPECT_EQ(values_columns[0], si0);
    EXPECT_EQ(values_columns[1], si1);
    EXPECT_EQ(values_columns[2], si2);

    auto&& relation = r.relation();
    ASSERT_FALSE(relation.declaration());

    EXPECT_EQ(relation.identifier(), "q");

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 3);
    {
        auto&& column = relation_columns[0];
        EXPECT_EQ(column.identifier(), "a");
        EXPECT_EQ(column.variable(), so0);
        EXPECT_EQ(column.declaration(), nullptr);
        EXPECT_TRUE(column.exported());
    }
    {
        auto&& column = relation_columns[1];
        EXPECT_EQ(column.identifier(), "b");
        EXPECT_EQ(column.variable(), so1);
        EXPECT_EQ(column.declaration(), nullptr);
        EXPECT_TRUE(column.exported());
    }
    {
        auto&& column = relation_columns[2];
        EXPECT_EQ(column.identifier(), "c");
        EXPECT_EQ(column.variable(), so2);
        EXPECT_EQ(column.declaration(), nullptr);
        EXPECT_TRUE(column.exported());
    }
}

TEST_F(analyze_query_expression_with_test, column_names_less) {
    invalid(sql_analyzer_code::inconsistent_columns, ast::query::with_expression {
            {
                ast::query::with_element {
                    id("q"),
                    {
                            id("a"),
                            id("b"),
                    },
                    ast::query::table_value_constructor {
                        ast::scalar::value_constructor {
                            literal(number("1")),
                            literal(number("2")),
                            literal(number("3")),
                        },
                    },
                },
            },
            ast::query::table_reference {
                    id("q"),
            },
    });
}

TEST_F(analyze_query_expression_with_test, column_names_more) {
    invalid(sql_analyzer_code::inconsistent_columns, ast::query::with_expression {
            {
                ast::query::with_element {
                    id("q"),
                    {
                            id("a"),
                            id("b"),
                            id("c"),
                            id("MORE"),
                    },
                    ast::query::table_value_constructor {
                        ast::scalar::value_constructor {
                            literal(number("1")),
                            literal(number("2")),
                            literal(number("3")),
                        },
                    },
                },
            },
            ast::query::table_reference {
                    id("q"),
            },
    });
}

TEST_F(analyze_query_expression_with_test, column_names_conflict) {
    invalid(sql_analyzer_code::column_already_exists, ast::query::with_expression {
            {
                ast::query::with_element {
                    id("q"),
                    {
                            id("a"),
                            id("b"),
                            id("a"), // conflict
                    },
                    ast::query::table_value_constructor {
                            ast::scalar::value_constructor {
                                    literal(number("1")),
                                    literal(number("2")),
                                    literal(number("3")),
                            },
                    },
                },
            },
            ast::query::table_reference {
                    id("q"),
            },
    });
}

TEST_F(analyze_query_expression_with_test, column_not_exported) {
    auto table = install_table("t");
    ::yugawara::storage::column_feature_set system_flags {
            ::yugawara::storage::column_feature::synthesized,
            ::yugawara::storage::column_feature::hidden,
    };
    table->columns()[2].features() += system_flags;
    table->columns()[3].features() += system_flags;

    trelation::graph_type graph {};
    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::with_expression {
                {
                    ast::query::with_element {
                        id("q"),
                        ast::query::table_reference {
                                id("t")
                        },
                    },
                },
                ast::query::table_reference {
                        id("q"),
                },
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(graph.size(), 1);
    EXPECT_FALSE(r.output().opposite());

    // subquery -
    auto&& subquery = downcast<::yugawara::extension::relation::subquery>(r.output().owner());

    ASSERT_EQ(subquery.mappings().size(), 2);
    auto&& si0 = subquery.mappings()[0].source();
    auto&& si1 = subquery.mappings()[1].source();
    auto&& so0 = subquery.mappings()[0].destination();
    auto&& so1 = subquery.mappings()[1].destination();

    auto&& subgraph = subquery.query_graph();
    ASSERT_EQ(subgraph.size(), 1);

    auto&& subquery_output = subquery.find_output_port();
    ASSERT_TRUE(subquery_output);
    ASSERT_FALSE(subquery_output->opposite());
    ASSERT_TRUE(subgraph.contains(subquery_output->owner()));

    // scan -
    auto&& values = downcast<trelation::scan>(subquery_output->owner());
    auto&& values_columns = values.columns();
    EXPECT_EQ(values_columns[0].destination(), si0);
    EXPECT_EQ(values_columns[1].destination(), si1);

    auto&& relation = r.relation();
    ASSERT_FALSE(relation.declaration());

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 2);
    {
        auto&& column = relation_columns[0];
        EXPECT_EQ(column.identifier(), "k");
        EXPECT_EQ(column.variable(), so0);
        EXPECT_EQ(column.declaration(), nullptr);
        EXPECT_TRUE(column.exported());
    }
    {
        auto&& column = relation_columns[1];
        EXPECT_EQ(column.identifier(), "v");
        EXPECT_EQ(column.variable(), so1);
        EXPECT_EQ(column.declaration(), nullptr);
        EXPECT_TRUE(column.exported());
    }
}

TEST_F(analyze_query_expression_with_test, invalid_recursive) {
    invalid(sql_analyzer_code::unsupported_feature, ast::query::with_expression {
            true,
            {
                ast::query::with_element {
                    id("q"),
                    ast::query::table_value_constructor {
                        ast::scalar::value_constructor {
                            literal(number("1")),
                        },
                    },
                },
            },
            ast::query::table_reference {
                    id("q"),
            },
    });
}

TEST_F(analyze_query_expression_with_test, multiple_elements) {
    trelation::graph_type graph {};
    auto r = analyze_query_expression(
            context(),
            graph,
            // WITH q0 AS (VALUES (0)),
            //      q1 AS (VALUES (1)),
            //      q2 AS (VALUES (2))
            // TABLE q0
            // UNION
            // TABLE q1
            // UNION
            // TABLE q2
            ast::query::with_expression {
                    {
                        ast::query::with_element {
                            id("q0"),
                            ast::query::table_value_constructor {
                                ast::scalar::value_constructor {
                                    literal(number("0")),
                                },
                            },
                        },
                        ast::query::with_element {
                            id("q1"),
                            ast::query::table_value_constructor {
                                ast::scalar::value_constructor {
                                    literal(number("1")),
                                },
                            },
                        },
                        ast::query::with_element {
                            id("q2"),
                            ast::query::table_value_constructor {
                                ast::scalar::value_constructor {
                                    literal(number("2")),
                                },
                            },
                        },
                    },
                    ast::query::binary_expression {
                            ast::query::binary_expression {
                                    ast::query::table_reference {
                                            id("q0"),
                                    },
                                    ast::query::binary_operator::union_,
                                    ast::query::table_reference {
                                            id("q1"),
                                    },
                            },
                            ast::query::binary_operator::union_,
                            ast::query::table_reference {
                                    id("q2"),
                            },
                    },
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(graph.size(), 5);
    EXPECT_FALSE(r.output().opposite());

    /*
     * subquery:q0 -\
     *               +-- union:u0 --\
     * subquery:q1 -/                \
     *                                +-- union:u1 --
     * subquery:q2 ------------------/
     */
    auto&& union_1 = downcast<trelation::intermediate::union_>(r.output().owner());
    auto&& union_0 = *find_prev<trelation::intermediate::union_>(union_1.left());
    auto&& subquery_0 = *find_prev<::yugawara::extension::relation::subquery>(union_0.left());
    auto&& subquery_1 = *find_prev<::yugawara::extension::relation::subquery>(union_0.right());
    auto&& subquery_2 = *find_prev<::yugawara::extension::relation::subquery>(union_1.right());

    ASSERT_EQ(subquery_0.mappings().size(), 1);
    auto&& s0o = subquery_0.mappings()[0].destination();

    ASSERT_EQ(subquery_1.mappings().size(), 1);
    auto&& s1o = subquery_1.mappings()[0].destination();

    ASSERT_EQ(subquery_2.mappings().size(), 1);
    auto&& s2o = subquery_2.mappings()[0].destination();

    ASSERT_EQ(union_0.mappings().size(), 1);
    EXPECT_EQ(union_0.mappings()[0].left(), s0o);
    EXPECT_EQ(union_0.mappings()[0].right(), s1o);
    auto&& u0o = union_0.mappings()[0].destination();

    ASSERT_EQ(union_1.mappings().size(), 1);
    EXPECT_EQ(union_1.mappings()[0].left(), u0o);
    EXPECT_EQ(union_1.mappings()[0].right(), s2o);
    auto&& u1o = union_1.mappings()[0].destination();

    auto&& relation = r.relation();
    ASSERT_FALSE(relation.declaration());

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 1);
    {
        auto&& column = relation_columns[0];
        EXPECT_EQ(column.identifier(), std::nullopt);
        EXPECT_EQ(column.variable(), u1o);
        EXPECT_EQ(column.declaration(), nullptr);
        EXPECT_TRUE(column.exported());
    }
}

TEST_F(analyze_query_expression_with_test, chain_elements) {
    trelation::graph_type graph {};
    auto r = analyze_query_expression(
            context(),
            graph,
            // WITH q0 AS (VALUES (0)),
            //      q1 AS (TABLE q0),
            //      q2 AS (TABLE q1)
            // TABLE q2
            ast::query::with_expression {
                {
                    ast::query::with_element {
                        id("q0"),
                        ast::query::table_value_constructor {
                            ast::scalar::value_constructor {
                                literal(number("0")),
                            },
                        },
                    },
                    ast::query::with_element {
                        id("q1"),
                        ast::query::table_reference {
                            id("q0"),
                        },
                    },
                    ast::query::with_element {
                        id("q2"),
                        ast::query::table_reference {
                            id("q1"),
                        }
                    },
                },
                ast::query::table_reference {
                        id("q2"),
                },
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(graph.size(), 1);
    EXPECT_FALSE(r.output().opposite());

    // subquery:q2 -
    auto&& subquery_2 = downcast<::yugawara::extension::relation::subquery>(r.output().owner());

    ASSERT_EQ(subquery_2.mappings().size(), 1);
    auto&& s2i = subquery_2.mappings()[0].source();
    auto&& s2o = subquery_2.mappings()[0].destination();

    auto&& graph_2 = subquery_2.query_graph();
    ASSERT_EQ(graph_2.size(), 1);

    auto&& suboutput_2 = subquery_2.find_output_port();
    ASSERT_TRUE(suboutput_2);

    // [q2]: subquery:q1 -
    auto&& subquery_1 = downcast<::yugawara::extension::relation::subquery>(suboutput_2->owner());

    ASSERT_EQ(subquery_1.mappings().size(), 1);
    auto&& s1i = subquery_1.mappings()[0].source();
    auto&& s1o = subquery_1.mappings()[0].destination();
    EXPECT_EQ(s1o, s2i);

    auto&& graph_1 = subquery_1.query_graph();
    ASSERT_EQ(graph_1.size(), 1);

    auto&& suboutput_1 = subquery_1.find_output_port();
    ASSERT_TRUE(suboutput_1);

    // [q1]: subquery:q0 -
    auto&& subquery_0 = downcast<::yugawara::extension::relation::subquery>(suboutput_1->owner());

    ASSERT_EQ(subquery_0.mappings().size(), 1);
    auto&& s0i = subquery_0.mappings()[0].source();
    auto&& s0o = subquery_0.mappings()[0].destination();
    EXPECT_EQ(s0o, s1i);

    auto&& graph_0 = subquery_0.query_graph();
    ASSERT_EQ(graph_0.size(), 1);

    auto&& suboutput_0 = subquery_0.find_output_port();
    ASSERT_TRUE(suboutput_0);

    // [q0]: values -
    auto&& values = downcast<trelation::values>(suboutput_0->owner());
    auto&& values_columns = values.columns();
    ASSERT_EQ(values_columns.size(), 1);
    EXPECT_EQ(values_columns[0], s0i);

    auto&& relation = r.relation();
    ASSERT_FALSE(relation.declaration());

    EXPECT_EQ(relation.identifier(), "q2");

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 1);
    {
        auto&& column = relation_columns[0];
        EXPECT_EQ(column.identifier(), std::nullopt);
        EXPECT_EQ(column.variable(), s2o);
        EXPECT_EQ(column.declaration(), nullptr);
        EXPECT_TRUE(column.exported());
    }
}

TEST_F(analyze_query_expression_with_test, conflict_query_names) {
    invalid(sql_analyzer_code::view_already_exists, ast::query::with_expression {
            {
                ast::query::with_element {
                    id("q0"),
                    ast::query::table_value_constructor {
                        ast::scalar::value_constructor {
                            literal(number("0")),
                        },
                    },
                },
                ast::query::with_element {
                    id("q1"),
                    ast::query::table_value_constructor {
                        ast::scalar::value_constructor {
                            literal(number("1")),
                        },
                    },
                },
                ast::query::with_element {
                    id("q1"), // conflict
                    ast::query::table_value_constructor {
                        ast::scalar::value_constructor {
                            literal(number("2")),
                        },
                    },
                },
            },
            ast::query::table_reference {
                    id("q0"),
            },
    });
}

TEST_F(analyze_query_expression_with_test, error_in_common_table_expression) {
    invalid(sql_analyzer_code::variable_not_found, ast::query::with_expression {
            {
                    ast::query::with_element {
                            id("q0"),
                            ast::query::table_value_constructor {
                                    ast::scalar::value_constructor {
                                            vref(id("INVALID")),
                                    },
                            },
                    },
            },
            ast::query::table_reference {
                    id("q0"),
            },
    });
}

} // namespace mizugaki::analyzer::details