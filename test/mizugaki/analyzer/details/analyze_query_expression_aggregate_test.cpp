#include <mizugaki/analyzer/details/analyze_query_expression.h>

#include <gtest/gtest.h>

#include <takatori/type/primitive.h>

#include <takatori/scalar/compare.h>

#include <takatori/relation/scan.h>
#include <takatori/relation/filter.h>
#include <takatori/relation/project.h>
#include <takatori/relation/intermediate/aggregate.h>
#include <takatori/relation/intermediate/limit.h>

#include <mizugaki/ast/scalar/comparison_predicate.h>
#include <mizugaki/ast/scalar/builtin_set_function_invocation.h>

#include <mizugaki/ast/query/query.h>
#include <mizugaki/ast/query/select_column.h>
#include <mizugaki/ast/query/grouping_column.h>

#include <mizugaki/ast/table/table_reference.h>

#include "test_parent.h"

namespace mizugaki::analyzer::details {

using namespace ::mizugaki::analyzer::testing;

class analyze_query_expression_aggregate_test : public test_parent {
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

    ::yugawara::binding::factory factory_;

    std::shared_ptr<::yugawara::aggregate::declaration> count_asterisk = set_functions_->add(
            ::yugawara::aggregate::declaration {
                    ::yugawara::aggregate::declaration::minimum_builtin_function_id + 1,
                    "count",
                    ttype::int8 {},
                    {},
                    true,
            });

    std::shared_ptr<::yugawara::aggregate::declaration> count_int = set_functions_->add(
            ::yugawara::aggregate::declaration {
                    ::yugawara::aggregate::declaration::minimum_builtin_function_id + 2,
                    "count",
                    ttype::int8 {},
                    {
                            ttype::int8 {},
                    },
                    true,
            });

    std::shared_ptr<::yugawara::aggregate::declaration> count_str = set_functions_->add(
            ::yugawara::aggregate::declaration {
                    ::yugawara::aggregate::declaration::minimum_builtin_function_id + 3,
                    "count",
                    ttype::int8 {},
                    {
                            ttype::character { ttype::varying },
                    },
                    true,
            });
};

TEST_F(analyze_query_expression_aggregate_test, whole_count_asterisk) {
    auto table = install_table("testing");
    trelation::graph_type graph {};

    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::query {
                    {
                            ast::query::select_column {
                                    ast::scalar::builtin_set_function_invocation {
                                            ast::scalar::builtin_set_function_kind::count,
                                            {},
                                            {},
                                    }
                            },
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

    // scan - aggregate - project -

    ASSERT_EQ(graph.size(), 3);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();

    auto&& project = downcast<trelation::project>(r.output().owner());
    auto&& aggregate = *find_prev<trelation::intermediate::aggregate>(project);
    auto&& scan = *find_prev<trelation::scan>(aggregate);

    auto&& scan_columns = scan.columns();
    ASSERT_EQ(scan_columns.size(), 4);

    auto&& grouping_columns = aggregate.group_keys();
    ASSERT_EQ(grouping_columns.size(), 0);

    auto&& aggregation_columns = aggregate.columns();
    ASSERT_EQ(aggregation_columns.size(), 1);
    {
        auto&& column = aggregation_columns[0];
        EXPECT_EQ(column.function(), factory_(count_asterisk));
        ASSERT_EQ(column.arguments().size(), 0);
    }

    auto&& project_columns = project.columns();
    ASSERT_EQ(project_columns.size(), 1);
    EXPECT_EQ(project_columns[0].value(), vref(aggregation_columns[0].destination()));

    // output

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 1);
    EXPECT_EQ(relation_columns[0].variable(), project_columns[0].variable());
}

TEST_F(analyze_query_expression_aggregate_test, whole_count_value) {
    auto table = install_table("testing");
    trelation::graph_type graph {};

    auto r = analyze_query_expression(
            context(),
            graph,
            // SELECT COUNT(v) FROM testing
            ast::query::query {
                    {
                            ast::query::select_column {
                                    ast::scalar::builtin_set_function_invocation {
                                            ast::scalar::builtin_set_function_kind::count,
                                            {},
                                            {
                                                    vref(id("v")),
                                            },
                                    }
                            },
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

    // scan - project - aggregate - project -

    ASSERT_EQ(graph.size(), 4);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();

    auto&& project = downcast<trelation::project>(r.output().owner());
    auto&& aggregate = *find_prev<trelation::intermediate::aggregate>(project);
    auto&& aggregate_args = *find_prev<trelation::project>(aggregate);
    auto&& scan = *find_prev<trelation::scan>(aggregate_args);

    auto&& scan_columns = scan.columns();
    ASSERT_EQ(scan_columns.size(), 4);

    auto&& aggargs_columns = aggregate_args.columns();
    ASSERT_EQ(aggargs_columns.size(), 1);
    EXPECT_EQ(aggargs_columns[0].value(), vref(scan_columns[1].destination()));

    auto&& grouping_columns = aggregate.group_keys();
    ASSERT_EQ(grouping_columns.size(), 0);

    auto&& aggregation_columns = aggregate.columns();
    ASSERT_EQ(aggregation_columns.size(), 1);
    {
        auto&& column = aggregation_columns[0];
        EXPECT_EQ(column.function(), factory_(count_str));
        ASSERT_EQ(column.arguments().size(), 1);
        EXPECT_EQ(column.arguments()[0], aggargs_columns[0].variable());
    }

    auto&& project_columns = project.columns();
    ASSERT_EQ(project_columns.size(), 1);
    EXPECT_EQ(project_columns[0].value(), vref(aggregation_columns[0].destination()));

    // output

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 1);
    EXPECT_EQ(relation_columns[0].variable(), project_columns[0].variable());
}

TEST_F(analyze_query_expression_aggregate_test, group_by) {
    auto table = install_table("testing");
    trelation::graph_type graph {};

    auto r = analyze_query_expression(
            context(),
            graph,
            ast::query::query {
                    {
                            ast::query::select_column {
                                    vref(id("v")),
                            },
                            ast::query::select_column {
                                    ast::scalar::builtin_set_function_invocation {
                                            ast::scalar::builtin_set_function_kind::count,
                                            {},
                                            {},
                                    }
                            },
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
            },
            {},
            {});
    ASSERT_TRUE(r);

    // scan - aggregate - project -

    ASSERT_EQ(graph.size(), 3);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();

    auto&& project = downcast<trelation::project>(r.output().owner());
    auto&& aggregate = *find_prev<trelation::intermediate::aggregate>(project);
    auto&& scan = *find_prev<trelation::scan>(aggregate);

    auto&& scan_columns = scan.columns();
    ASSERT_EQ(scan_columns.size(), 4);

    auto&& grouping_columns = aggregate.group_keys();
    ASSERT_EQ(grouping_columns.size(), 1);
    EXPECT_EQ(grouping_columns[0], scan_columns[1].destination());

    auto&& aggregation_columns = aggregate.columns();
    ASSERT_EQ(aggregation_columns.size(), 1);
    {
        auto&& column = aggregation_columns[0];
        EXPECT_EQ(column.function(), factory_(count_asterisk));
        ASSERT_EQ(column.arguments().size(), 0);
    }

    auto&& project_columns = project.columns();
    ASSERT_EQ(project_columns.size(), 2);
    EXPECT_EQ(project_columns[0].value(), vref(scan_columns[1].destination()));
    EXPECT_EQ(project_columns[1].value(), vref(aggregation_columns[0].destination()));

    // output

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 2);
    EXPECT_EQ(relation_columns[0].variable(), project_columns[0].variable());
    EXPECT_EQ(relation_columns[1].variable(), project_columns[1].variable());
}

TEST_F(analyze_query_expression_aggregate_test, group_by_having) {
    auto table = install_table("testing");
    trelation::graph_type graph {};

    auto r = analyze_query_expression(
            context(),
            graph,
            // SELECT v FROM testing GROUP BY v HAVING COUNT(*) > 1
            ast::query::query {
                    {
                            ast::query::select_column {
                                    vref(id("v")),
                            },
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
                                    ast::scalar::builtin_set_function_invocation {
                                            ast::scalar::builtin_set_function_kind::count,
                                            {},
                                            {},
                                    },
                                    ast::scalar::comparison_operator::greater_than,
                                    literal(number("1")),
                            },
                    },
            },
            {},
            {});
    ASSERT_TRUE(r);

    // scan - aggregate - filter - project -

    ASSERT_EQ(graph.size(), 4);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();

    auto&& project = downcast<trelation::project>(r.output().owner());
    auto&& filter = *find_prev<trelation::filter>(project);
    auto&& aggregate = *find_prev<trelation::intermediate::aggregate>(filter);
    auto&& scan = *find_prev<trelation::scan>(aggregate);

    auto&& scan_columns = scan.columns();
    ASSERT_EQ(scan_columns.size(), 4);

    auto&& grouping_columns = aggregate.group_keys();
    ASSERT_EQ(grouping_columns.size(), 1);
    EXPECT_EQ(grouping_columns[0], scan_columns[1].destination());

    auto&& aggregation_columns = aggregate.columns();
    ASSERT_EQ(aggregation_columns.size(), 1);
    {
        auto&& column = aggregation_columns[0];
        EXPECT_EQ(column.function(), factory_(count_asterisk));
        ASSERT_EQ(column.arguments().size(), 0);
    }

    EXPECT_EQ(filter.condition(), (tscalar::compare {
            tscalar::comparison_operator::greater,
            vref(aggregation_columns[0].destination()),
            immediate(1),
    }));

    auto&& project_columns = project.columns();
    ASSERT_EQ(project_columns.size(), 1);
    EXPECT_EQ(project_columns[0].value(), vref(scan_columns[1].destination()));

    // output

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 1);
    EXPECT_EQ(relation_columns[0].variable(), project_columns[0].variable());
}

TEST_F(analyze_query_expression_aggregate_test, group_by_order_by) {
    auto table = install_table("testing");
    trelation::graph_type graph {};

    auto r = analyze_query_expression(
            context(),
            graph,
            // SELECT v FROM testing GROUP BY v ORDER BY COUNT(*)
            ast::query::query {
                    {
                            ast::query::select_column {
                                    vref(id("v")),
                            },
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
                    {},
                    {
                            ast::scalar::builtin_set_function_invocation {
                                    ast::scalar::builtin_set_function_kind::count,
                                    {},
                                    {},
                            },
                    },
            },
            {},
            {});
    ASSERT_TRUE(r);

    // scan - aggregate - project - project - limit -

    ASSERT_EQ(graph.size(), 5);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();

    auto&& limit = downcast<trelation::intermediate::limit>(r.output().owner());
    auto&& limitargs = *find_prev<trelation::project>(limit);
    auto&& project = *find_prev<trelation::project>(limitargs);
    auto&& aggregate = *find_prev<trelation::intermediate::aggregate>(project);
    auto&& scan = *find_prev<trelation::scan>(aggregate);

    auto&& scan_columns = scan.columns();
    ASSERT_EQ(scan_columns.size(), 4);

    auto&& grouping_columns = aggregate.group_keys();
    ASSERT_EQ(grouping_columns.size(), 1);
    EXPECT_EQ(grouping_columns[0], scan_columns[1].destination());

    auto&& aggregation_columns = aggregate.columns();
    ASSERT_EQ(aggregation_columns.size(), 1);
    {
        auto&& column = aggregation_columns[0];
        EXPECT_EQ(column.function(), factory_(count_asterisk));
        ASSERT_EQ(column.arguments().size(), 0);
    }

    auto&& project_columns = project.columns();
    ASSERT_EQ(project_columns.size(), 1);
    EXPECT_EQ(project_columns[0].value(), vref(scan_columns[1].destination()));

    auto&& limit_columns = limitargs.columns();
    ASSERT_EQ(limit_columns.size(), 1);
    EXPECT_EQ(limit_columns[0].value(), vref(aggregation_columns[0].destination()));

    EXPECT_EQ(limit.count(), std::nullopt);
    ASSERT_EQ(limit.group_keys().size(), 0);
    ASSERT_EQ(limit.sort_keys().size(), 1);
    EXPECT_EQ(limit.sort_keys()[0], limit_columns[0].variable());

    // output

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 1);
    EXPECT_EQ(relation_columns[0].variable(), project_columns[0].variable());
}

TEST_F(analyze_query_expression_aggregate_test, group_by_where) {
    auto table = install_table("testing");
    trelation::graph_type graph {};

    auto r = analyze_query_expression(
            context(),
            graph,
            // SELECT v FROM testing WHERE k >= 0 GROUP BY v
            ast::query::query {
                    {
                            ast::query::select_column {
                                    vref(id("v")),
                            },
                            ast::query::select_column {
                                    ast::scalar::builtin_set_function_invocation {
                                            ast::scalar::builtin_set_function_kind::count,
                                            {},
                                            {},
                                    }
                            },
                    },
                    {
                            ast::table::table_reference {
                                    id("testing"),
                            }
                    },
                    {
                            ast::scalar::comparison_predicate {
                                    vref(id("k")),
                                    ast::scalar::comparison_operator::greater_than_or_equals,
                                    literal(number("0")),
                            },
                    },
                    ast::query::group_by_clause {
                            ast::query::grouping_column { id("v") },
                    },
            },
            {},
            {});
    ASSERT_TRUE(r);

    // scan - filter - aggregate - project -

    ASSERT_EQ(graph.size(), 4);
    EXPECT_FALSE(r.output().opposite());

    auto&& relation = r.relation();

    auto&& project = downcast<trelation::project>(r.output().owner());
    auto&& aggregate = *find_prev<trelation::intermediate::aggregate>(project);
    auto&& filter = *find_prev<trelation::filter>(aggregate);
    auto&& scan = *find_prev<trelation::scan>(filter);

    auto&& scan_columns = scan.columns();
    ASSERT_EQ(scan_columns.size(), 4);

    EXPECT_EQ(filter.condition(), (tscalar::compare {
            tscalar::comparison_operator::greater_equal,
            vref(scan_columns[0].destination()),
            immediate(0),

    }));

    auto&& grouping_columns = aggregate.group_keys();
    ASSERT_EQ(grouping_columns.size(), 1);
    EXPECT_EQ(grouping_columns[0], scan_columns[1].destination());

    auto&& aggregation_columns = aggregate.columns();
    ASSERT_EQ(aggregation_columns.size(), 1);
    {
        auto&& column = aggregation_columns[0];
        EXPECT_EQ(column.function(), factory_(count_asterisk));
        ASSERT_EQ(column.arguments().size(), 0);
    }

    auto&& project_columns = project.columns();
    ASSERT_EQ(project_columns.size(), 2);
    EXPECT_EQ(project_columns[0].value(), vref(scan_columns[1].destination()));
    EXPECT_EQ(project_columns[1].value(), vref(aggregation_columns[0].destination()));

    // output

    auto relation_columns = relation.columns();
    ASSERT_EQ(relation_columns.size(), 2);
    EXPECT_EQ(relation_columns[0].variable(), project_columns[0].variable());
    EXPECT_EQ(relation_columns[1].variable(), project_columns[1].variable());
}

TEST_F(analyze_query_expression_aggregate_test, select_invalid_column) {
    auto table = install_table("testing");
    invalid(sql_analyzer_code::invalid_aggregation_column, ast::query::query {
            {
                    ast::query::select_column {
                            vref(id("k")),
                    },
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
    });
}

TEST_F(analyze_query_expression_aggregate_test, group_by_invalid_column_expression) {
    auto table = install_table("testing");
    invalid(ast::query::query {
            {
                    ast::query::select_column {
                            vref(id("v")),
                    },
                    ast::query::select_column {
                            ast::scalar::builtin_set_function_invocation {
                                    ast::scalar::builtin_set_function_kind::count,
                                    {},
                                    {},
                            }
                    },
            },
            {
                    ast::table::table_reference {
                            id("testing"),
                    }
            },
            {},
            ast::query::group_by_clause {
                    ast::query::grouping_column { id("MISSING") },
            },
    });
}

TEST_F(analyze_query_expression_aggregate_test, group_by_invalid_column_aggregated) {
    auto table = install_table("testing");
    invalid(sql_analyzer_code::unsupported_feature, ast::query::query {
            {
                    ast::query::select_column {
                            vref(id("v")),
                    },
                    ast::query::select_column {
                            ast::scalar::builtin_set_function_invocation {
                                    ast::scalar::builtin_set_function_kind::count,
                                    {},
                                    {},
                            }
                    },
            },
            {
                    ast::table::table_reference {
                            id("testing"),
                    }
            },
            {},
            ast::query::group_by_clause {
                    ast::query::grouping_column {
                            ast::scalar::builtin_set_function_invocation {
                                    ast::scalar::builtin_set_function_kind::count,
                                    {},
                                    {},
                            }
                    },
            },
    });
}

TEST_F(analyze_query_expression_aggregate_test, group_by_invalid_column_not_variable) {
    auto table = install_table("testing");
    invalid(sql_analyzer_code::unsupported_feature, ast::query::query {
            {
                    ast::query::select_column {
                            vref(id("v")),
                    },
                    ast::query::select_column {
                            ast::scalar::builtin_set_function_invocation {
                                    ast::scalar::builtin_set_function_kind::count,
                                    {},
                                    {},
                            }
                    },
            },
            {
                    ast::table::table_reference {
                            id("testing"),
                    }
            },
            {},
            ast::query::group_by_clause {
                    ast::query::grouping_column { literal(number("1")) },
            },
    });
}

TEST_F(analyze_query_expression_aggregate_test, having_invalid_column) {
    auto table = install_table("testing");
    invalid(sql_analyzer_code::invalid_aggregation_column, ast::query::query {
            {
                    ast::query::select_column {
                            vref(id("v")),
                    },
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
                            vref(id("k")),
                            ast::scalar::comparison_operator::greater_than,
                            literal(number("1")),
                    },
            },
    });
}

TEST_F(analyze_query_expression_aggregate_test, order_by_invalid_column) {
    auto table = install_table("testing");
    invalid(sql_analyzer_code::invalid_aggregation_column, ast::query::query {
            {
                    ast::query::select_column {
                            vref(id("v")),
                    },
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
            {},
            {
                    vref(id("k")),
            }
    });
}

TEST_F(analyze_query_expression_aggregate_test, where_aggregated) {
    auto table = install_table("testing");
    invalid(sql_analyzer_code::unsupported_feature, ast::query::query {
            {
                    ast::query::select_column {
                            vref(id("v")),
                    },
                    ast::query::select_column {
                            ast::scalar::builtin_set_function_invocation {
                                    ast::scalar::builtin_set_function_kind::count,
                                    {},
                                    {},
                            }
                    },
            },
            {
                    ast::table::table_reference {
                            id("testing"),
                    }
            },
            {
                    ast::scalar::comparison_predicate {
                            ast::scalar::builtin_set_function_invocation {
                                    ast::scalar::builtin_set_function_kind::count,
                                    {},
                                    {},
                            },
                            ast::scalar::comparison_operator::greater_than,
                            literal(number("1")),
                    },
            },
    });
}

} // namespace mizugaki::analyzer::details
