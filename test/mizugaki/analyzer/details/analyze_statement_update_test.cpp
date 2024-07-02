#include <mizugaki/analyzer/details/analyze_statement.h>

#include <gtest/gtest.h>

#include <takatori/relation/emit.h>
#include <takatori/relation/filter.h>
#include <takatori/relation/project.h>
#include <takatori/relation/scan.h>
#include <takatori/relation/write.h>

#include <takatori/statement/write.h>

#include <yugawara/binding/extract.h>

#include <yugawara/storage/index.h>
#include <yugawara/storage/column.h>

#include <mizugaki/ast/literal/boolean.h>

#include <mizugaki/ast/scalar/comparison_predicate.h>
#include <mizugaki/ast/scalar/builtin_set_function_invocation.h>

#include <mizugaki/ast/statement/empty_statement.h>
#include <mizugaki/ast/statement/update_statement.h>

#include "test_parent.h"

namespace mizugaki::analyzer::details {

using namespace ::mizugaki::analyzer::testing;

using ::yugawara::binding::extract;

class analyze_statement_update_test : public test_parent {
protected:
    void invalid(ast::statement::statement const& stmt) {
        trelation::graph_type graph {};
        auto r = analyze_statement(context(), stmt);
        EXPECT_TRUE(std::holds_alternative<erroneous_result_type>(r)) << diagnostics();
        EXPECT_GT(count_error(), 0);
    }

    void invalid(sql_analyzer_code code, ast::statement::statement const& stmt) {
        invalid(stmt);
        EXPECT_TRUE(find_error(code)) << diagnostics();
    }
};

TEST_F(analyze_statement_update_test, simple) {
    auto table = install_table("testing");

    auto r = analyze_statement(context(), ast::statement::update_statement {
            id("testing"),
            {
                    {
                            id("v"),
                            literal(string("'V2'")),
                    },
            },
    });
    auto alternative = std::get_if<execution_plan_result_type >(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    auto&& graph = **alternative;
    ASSERT_EQ(graph.size(), 3);

    auto first = find_first<trelation::scan>(graph);
    ASSERT_TRUE(first);

    EXPECT_EQ(&extract<::yugawara::storage::index>(first->source()).table(), table.get());
    auto&& scan_columns = first->columns();
    ASSERT_EQ(scan_columns.size(), 4);
    std::vector<::takatori::descriptor::variable> variables {};
    {
        auto&& column = scan_columns[0];
        EXPECT_EQ(&extract<::yugawara::storage::column>(column.source()), &table->columns()[0]);
        variables.emplace_back(column.destination());
    }
    {
        auto&& column = scan_columns[1];
        EXPECT_EQ(&extract<::yugawara::storage::column>(column.source()), &table->columns()[1]);
        variables.emplace_back(column.destination());
    }
    {
        auto&& column = scan_columns[2];
        EXPECT_EQ(&extract<::yugawara::storage::column>(column.source()), &table->columns()[2]);
        variables.emplace_back(column.destination());
    }
    {
        auto&& column = scan_columns[3];
        EXPECT_EQ(&extract<::yugawara::storage::column>(column.source()), &table->columns()[3]);
        variables.emplace_back(column.destination());
    }
    EXPECT_EQ(first->lower(), trelation::scan::endpoint());
    EXPECT_EQ(first->upper(), trelation::scan::endpoint());
    EXPECT_EQ(first->limit(), std::nullopt);

    auto project = find_next<trelation::project>(*first);
    ASSERT_TRUE(project);
    ASSERT_EQ(project->columns().size(), 1);
    {
        auto&& column = project->columns()[0];
        EXPECT_EQ(column.value(), immediate("V2"));
        variables.emplace_back(column.variable());
    }

    auto last = find_last<trelation::write>(graph);
    ASSERT_TRUE(last);
    EXPECT_EQ(find_next(*project).get(), last.get());

    EXPECT_EQ(last->operator_kind(), tstatement::write_kind::update);
    EXPECT_EQ(&extract<::yugawara::storage::index>(last->destination()).table(), table.get());

    ASSERT_EQ(last->keys().size(), 1);
    {
        auto&& column = last->keys()[0];
        EXPECT_EQ(column.source(), variables[0]);
        EXPECT_EQ(&extract<::yugawara::storage::column>(column.destination()), &table->columns()[0]);
    }

    ASSERT_EQ(last->columns().size(), 1);
    {
        auto&& column = last->columns()[0];
        EXPECT_EQ(column.source(), variables[4]);
        EXPECT_EQ(&extract<::yugawara::storage::column>(column.destination()), &table->columns()[1]);
    }
}

TEST_F(analyze_statement_update_test, modify_key) {
    auto table = install_table("testing");

    // UPADTE testing SET k = 2
    auto r = analyze_statement(context(), ast::statement::update_statement {
            id("testing"),
            {
                    {
                            id("k"),
                            literal(number("2")),
                    },
            },
    });
    auto alternative = std::get_if<execution_plan_result_type >(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    auto&& graph = **alternative;

    // scan - project[k'=2] - write[[k=k'], []]
    ASSERT_EQ(graph.size(), 3);

    auto first = find_first<trelation::scan>(graph);
    ASSERT_TRUE(first);

    EXPECT_EQ(&extract<::yugawara::storage::index>(first->source()).table(), table.get());
    auto&& scan_columns = first->columns();
    ASSERT_EQ(scan_columns.size(), 4);
    std::vector<::takatori::descriptor::variable> variables {};
    {
        auto&& column = scan_columns[0];
        EXPECT_EQ(&extract<::yugawara::storage::column>(column.source()), &table->columns()[0]);
        variables.emplace_back(column.destination());
    }
    {
        auto&& column = scan_columns[1];
        EXPECT_EQ(&extract<::yugawara::storage::column>(column.source()), &table->columns()[1]);
        variables.emplace_back(column.destination());
    }
    {
        auto&& column = scan_columns[2];
        EXPECT_EQ(&extract<::yugawara::storage::column>(column.source()), &table->columns()[2]);
        variables.emplace_back(column.destination());
    }
    {
        auto&& column = scan_columns[3];
        EXPECT_EQ(&extract<::yugawara::storage::column>(column.source()), &table->columns()[3]);
        variables.emplace_back(column.destination());
    }
    EXPECT_EQ(first->lower(), trelation::scan::endpoint());
    EXPECT_EQ(first->upper(), trelation::scan::endpoint());
    EXPECT_EQ(first->limit(), std::nullopt);

    // k'=2
    auto project = find_next<trelation::project>(*first);
    ASSERT_TRUE(project);
    auto&& project_columns = project->columns();
    ASSERT_EQ(project->columns().size(), 1);
    {
        auto&& column = project_columns[0];
        EXPECT_EQ(column.value(), immediate(2));
    }

    // write[[k=k'], []]
    auto last = find_last<trelation::write>(graph);
    ASSERT_TRUE(last);
    EXPECT_EQ(find_next(*project).get(), last.get());

    EXPECT_EQ(last->operator_kind(), tstatement::write_kind::update);
    EXPECT_EQ(&extract<::yugawara::storage::index>(last->destination()).table(), table.get());

    ASSERT_EQ(last->keys().size(), 1);
    {
        auto&& column = last->keys()[0];
        EXPECT_EQ(column.source(), scan_columns[0].destination());
        EXPECT_EQ(&extract<::yugawara::storage::column>(column.destination()), &table->columns()[0]);
    }

    ASSERT_EQ(last->columns().size(), 1);
    {
        auto&& column = last->columns()[0];
        EXPECT_EQ(column.source(), project_columns[0].variable());
        EXPECT_EQ(&extract<::yugawara::storage::column>(column.destination()), &table->columns()[0]);
    }
}

TEST_F(analyze_statement_update_test, multiple_set) {
    auto table = install_table("testing");

    auto r = analyze_statement(context(), ast::statement::update_statement {
            id("testing"),
            {
                    { id("v"), vref(id("w")), },
                    { id("w"), vref(id("x")), },
                    { id("x"), vref(id("v")), },
            },
    });
    auto alternative = std::get_if<execution_plan_result_type >(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    auto&& graph = **alternative;
    ASSERT_EQ(graph.size(), 3);

    auto first = find_first<trelation::scan>(graph);
    ASSERT_TRUE(first);

    EXPECT_EQ(&extract<::yugawara::storage::index>(first->source()).table(), table.get());
    auto&& scan_columns = first->columns();
    ASSERT_EQ(scan_columns.size(), 4);
    std::vector<::takatori::descriptor::variable> variables {};
    {
        auto&& column = scan_columns[0];
        EXPECT_EQ(&extract<::yugawara::storage::column>(column.source()), &table->columns()[0]);
        variables.emplace_back(column.destination());
    }
    {
        auto&& column = scan_columns[1];
        EXPECT_EQ(&extract<::yugawara::storage::column>(column.source()), &table->columns()[1]);
        variables.emplace_back(column.destination());
    }
    {
        auto&& column = scan_columns[2];
        EXPECT_EQ(&extract<::yugawara::storage::column>(column.source()), &table->columns()[2]);
        variables.emplace_back(column.destination());
    }
    {
        auto&& column = scan_columns[3];
        EXPECT_EQ(&extract<::yugawara::storage::column>(column.source()), &table->columns()[3]);
        variables.emplace_back(column.destination());
    }
    EXPECT_EQ(first->lower(), trelation::scan::endpoint());
    EXPECT_EQ(first->upper(), trelation::scan::endpoint());
    EXPECT_EQ(first->limit(), std::nullopt);

    auto project = find_next<trelation::project>(*first);
    ASSERT_TRUE(project);
    ASSERT_EQ(project->columns().size(), 3);
    {
        auto&& column = project->columns()[0];
        EXPECT_EQ(column.value(), vref(variables[2]));
        variables.emplace_back(column.variable());
    }
    {
        auto&& column = project->columns()[1];
        EXPECT_EQ(column.value(), vref(variables[3]));
        variables.emplace_back(column.variable());
    }
    {
        auto&& column = project->columns()[2];
        EXPECT_EQ(column.value(), vref(variables[1]));
        variables.emplace_back(column.variable());
    }

    auto last = find_last<trelation::write>(graph);
    ASSERT_TRUE(last);
    EXPECT_EQ(find_next(*project).get(), last.get());

    EXPECT_EQ(last->operator_kind(), tstatement::write_kind::update);
    EXPECT_EQ(&extract<::yugawara::storage::index>(last->destination()).table(), table.get());

    ASSERT_EQ(last->keys().size(), 1);
    {
        auto&& column = last->keys()[0];
        EXPECT_EQ(column.source(), variables[0]);
        EXPECT_EQ(&extract<::yugawara::storage::column>(column.destination()), &table->columns()[0]);
    }

    ASSERT_EQ(last->columns().size(), 3);
    {
        auto&& column = last->columns()[0];
        EXPECT_EQ(column.source(), variables[4]);
        EXPECT_EQ(&extract<::yugawara::storage::column>(column.destination()), &table->columns()[1]);
    }
    {
        auto&& column = last->columns()[1];
        EXPECT_EQ(column.source(), variables[5]);
        EXPECT_EQ(&extract<::yugawara::storage::column>(column.destination()), &table->columns()[2]);
    }
    {
        auto&& column = last->columns()[2];
        EXPECT_EQ(column.source(), variables[6]);
        EXPECT_EQ(&extract<::yugawara::storage::column>(column.destination()), &table->columns()[3]);
    }
}

TEST_F(analyze_statement_update_test, where) {
    auto table = install_table("testing");

    auto r = analyze_statement(context(), ast::statement::update_statement {
            id("testing"),
            {
                    {
                            id("v"),
                            literal(string("'filter'"))
                    },
            },
            literal(ast::literal::boolean { true }),
    });
    auto alternative = std::get_if<execution_plan_result_type >(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    auto&& graph = **alternative;
    ASSERT_EQ(graph.size(), 4);

    auto first = find_first<trelation::scan>(graph);
    ASSERT_TRUE(first);

    auto filter = find_next<trelation::filter>(*first);
    ASSERT_TRUE(filter);
    ASSERT_EQ(filter->condition(), immediate_bool(true));

    auto project = find_next<trelation::project>(*filter);
    ASSERT_TRUE(project);

    auto last = find_last<trelation::write>(graph);
    ASSERT_TRUE(last);
    EXPECT_EQ(find_next(*project).get(), last.get());
}

TEST_F(analyze_statement_update_test, invalid_relation) {
    invalid(ast::statement::update_statement{
            id("MISSING"),
            {
                    {
                            id("v"),
                            literal(string("'X'")),
                    },
            },
    });
}

TEST_F(analyze_statement_update_test, columns_empty) {
    auto table = install_table("testing");
    invalid(sql_analyzer_code::malformed_syntax, ast::statement::update_statement{
            id("testing"),
            {},
    });
}

TEST_F(analyze_statement_update_test, columns_not_found) {
    auto table = install_table("testing");
    invalid(sql_analyzer_code::column_not_found, ast::statement::update_statement{
            id("testing"),
            {
                    {
                            id("MISSING"),
                            literal(string("'X'")),
                    },
            },
    });
}

TEST_F(analyze_statement_update_test, columns_duplicated) {
    auto table = install_table("testing");
    invalid(sql_analyzer_code::column_already_exists, ast::statement::update_statement{
            id("testing"),
            {
                    {
                            id("v"),
                            literal(string("'V'")),
                    },
                    {
                            id("w"),
                            literal(string("'W'")),
                    },
                    {
                            id("x"),
                            literal(string("'X'")),
                    },
                    {
                            id("v"),
                            literal(string("'X'")),
                    },
            },
    });
}

TEST_F(analyze_statement_update_test, columns_invalid_expression) {
    auto table = install_table("testing");
    invalid(ast::statement::update_statement{
            id("testing"),
            {
                    {
                            id("x"),
                            vref(id("MISSING")),
                    },
            },
    });
}

TEST_F(analyze_statement_update_test, invalid_where) {
    auto table = install_table("testing");
    invalid(ast::statement::update_statement{
            id("testing"),
            {
                    {
                            id("v"),
                            literal(string("'X'")),
                    },
            },
            {
                    ast::scalar::comparison_predicate {
                            vref(id("MISSING")),
                            ast::scalar::comparison_operator::greater_than,
                            literal(number("0")),
                    },
            },
    });
}

TEST_F(analyze_statement_update_test, invalid_where_aggregated) {
    auto table = install_table("testing");
    auto count = set_functions_->add(::yugawara::aggregate::declaration {
            ::yugawara::aggregate::declaration::minimum_builtin_function_id + 1,
            "count",
            ttype::int8 {},
            {},
            true,
    });
    invalid(sql_analyzer_code::unsupported_feature, ast::statement::update_statement{
            id("testing"),
            {
                    {
                            id("v"),
                            literal(string("'X'")),
                    },
            },
            {
                    ast::scalar::comparison_predicate {
                            ast::scalar::builtin_set_function_invocation {
                                    ast::scalar::builtin_set_function_kind::count,
                                    {},
                                    {},
                            },
                            ast::scalar::comparison_operator::greater_than,
                            literal(number("0")),
                    },
            },
    });
}

} // namespace mizugaki::analyzer::details
