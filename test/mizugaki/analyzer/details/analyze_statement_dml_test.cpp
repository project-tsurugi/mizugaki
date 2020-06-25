#include <mizugaki/analyzer/details/analyze_statement.h>

#include <gtest/gtest.h>

#include <takatori/value/primitive.h>
#include <takatori/type/primitive.h>

#include <takatori/relation/emit.h>
#include <takatori/relation/filter.h>
#include <takatori/relation/project.h>
#include <takatori/relation/scan.h>
#include <takatori/relation/values.h>
#include <takatori/relation/write.h>

#include <takatori/statement/empty.h>
#include <takatori/statement/write.h>

#include <yugawara/binding/factory.h>
#include <yugawara/binding/extract.h>

#include <yugawara/storage/table.h>
#include <yugawara/storage/index.h>
#include <yugawara/storage/column.h>

#include <mizugaki/ast/literal/special.h>
#include <mizugaki/ast/literal/boolean.h>

#include <mizugaki/ast/scalar/value_constructor.h>

#include <mizugaki/ast/query/table_reference.h>
#include <mizugaki/ast/query/table_value_constructor.h>

#include <mizugaki/ast/statement/empty_statement.h>
#include <mizugaki/ast/statement/select_statement.h>
#include <mizugaki/ast/statement/insert_statement.h>
#include <mizugaki/ast/statement/update_statement.h>
#include <mizugaki/ast/statement/delete_statement.h>

#include "test_parent.h"

namespace mizugaki::analyzer::details {

using namespace ::mizugaki::analyzer::testing;

using ::yugawara::binding::extract;

class analyze_statement_dml_test : public test_parent {};

// TODO: more tests for anomaly cases

TEST_F(analyze_statement_dml_test, empty_statement) {
    auto r = analyze_statement(context(), ast::statement::empty_statement {});
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative);
    auto&& stmt = *alternative;
    ASSERT_EQ(stmt->kind(), tstatement::statement_kind::empty);
}

TEST_F(analyze_statement_dml_test, select_statement_simple) {
    auto table = install_table("testing");
    auto r = analyze_statement(context(), ast::statement::select_statement {
            ast::query::table_reference {
                    id("testing"),
            },
    });
    auto alternative = std::get_if<execution_plan_result_type >(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    auto&& graph = **alternative;
    ASSERT_EQ(graph.size(), 2);

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

    auto last = find_last<trelation::emit>(graph);
    ASSERT_TRUE(last);
    EXPECT_EQ(find_next(*first).get(), last.get());
    auto&& emit_columns = last->columns();
    ASSERT_EQ(emit_columns.size(), 4);
    {
        auto&& column = emit_columns[0];
        EXPECT_EQ(column.source(), variables[0]);
        EXPECT_EQ(column.name(), table->columns()[0].simple_name());
    }
}

TEST_F(analyze_statement_dml_test, insert_statement_simple) {
    options_.prefer_write_statement() = false;
    auto table = install_table("testing");

    auto r = analyze_statement(context(), ast::statement::insert_statement {
            id("testing"),
            {},
            ast::query::table_value_constructor {
                    ast::scalar::value_constructor {
                            literal(number("1")),
                            literal(string("'a'")),
                            literal(string("'b'")),
                            literal(string("'c'")),
                    }
            },
    });
    auto alternative = std::get_if<execution_plan_result_type >(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    auto&& graph = **alternative;
    ASSERT_EQ(graph.size(), 2);

    auto first = find_first<trelation::values>(graph);
    ASSERT_TRUE(first);
    ASSERT_EQ(first->columns().size(), 4);
    ASSERT_EQ(first->rows().size(), 1);
    ASSERT_EQ(first->rows()[0].elements().size(), 4);
    {
        auto&& row = first->rows()[0].elements();
        EXPECT_EQ(row[0], immediate(1));
        EXPECT_EQ(row[1], immediate("a"));
        EXPECT_EQ(row[2], immediate("b"));
        EXPECT_EQ(row[3], immediate("c"));
    }

    auto last = find_last<trelation::write>(graph);
    ASSERT_TRUE(last);
    EXPECT_EQ(find_next(*first).get(), last.get());

    EXPECT_EQ(last->operator_kind(), tstatement::write_kind::insert);
    EXPECT_EQ(&extract<::yugawara::storage::index>(last->destination()).table(), table.get());

    ASSERT_EQ(last->keys().size(), 1);
    {
        auto&& column = last->keys()[0];
        EXPECT_EQ(column.source(), first->columns()[0]);
        EXPECT_EQ(&extract<::yugawara::storage::column>(column.destination()), &table->columns()[0]);
    }

    ASSERT_EQ(last->columns().size(), 3);
    {
        auto&& column = last->columns()[0];
        EXPECT_EQ(column.source(), first->columns()[1]);
        EXPECT_EQ(&extract<::yugawara::storage::column>(column.destination()), &table->columns()[1]);
    }
    {
        auto&& column = last->columns()[1];
        EXPECT_EQ(column.source(), first->columns()[2]);
        EXPECT_EQ(&extract<::yugawara::storage::column>(column.destination()), &table->columns()[2]);
    }
    {
        auto&& column = last->columns()[2];
        EXPECT_EQ(column.source(), first->columns()[3]);
        EXPECT_EQ(&extract<::yugawara::storage::column>(column.destination()), &table->columns()[3]);
    }
}

TEST_F(analyze_statement_dml_test, insert_statement_partial) {
    options_.prefer_write_statement() = false;
    auto table = install_table("testing");

    auto r = analyze_statement(context(), ast::statement::insert_statement {
            id("testing"),
                    {
                            id("v"),
                            id("k"),
                    },
            ast::query::table_value_constructor {
                    ast::scalar::value_constructor {
                            literal(string("'a'")),
                            literal(number("1")),
                    }
            },
    });
    auto alternative = std::get_if<execution_plan_result_type >(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    auto&& graph = **alternative;
    ASSERT_EQ(graph.size(), 2);

    auto first = find_first<trelation::values>(graph);
    ASSERT_TRUE(first);
    ASSERT_EQ(first->columns().size(), 2);
    ASSERT_EQ(first->rows().size(), 1);
    ASSERT_EQ(first->rows()[0].elements().size(), 2);
    {
        auto&& row = first->rows()[0].elements();
        EXPECT_EQ(row[0], immediate("a"));
        EXPECT_EQ(row[1], immediate(1));
    }

    auto last = find_last<trelation::write>(graph);
    ASSERT_TRUE(last);
    EXPECT_EQ(find_next(*first).get(), last.get());

    EXPECT_EQ(last->operator_kind(), tstatement::write_kind::insert);
    EXPECT_EQ(&extract<::yugawara::storage::index>(last->destination()).table(), table.get());

    ASSERT_EQ(last->keys().size(), 1);
    {
        auto&& column = last->keys()[0];
        EXPECT_EQ(column.source(), first->columns()[1]);
        EXPECT_EQ(&extract<::yugawara::storage::column>(column.destination()), &table->columns()[0]);
    }

    ASSERT_EQ(last->columns().size(), 1);
    {
        auto&& column = last->columns()[0];
        EXPECT_EQ(column.source(), first->columns()[0]);
        EXPECT_EQ(&extract<::yugawara::storage::column>(column.destination()), &table->columns()[1]);
    }
}

TEST_F(analyze_statement_dml_test, insert_statement_statement) {
    options_.prefer_write_statement() = true;
    auto table = install_table("testing");

    auto r = analyze_statement(context(), ast::statement::insert_statement {
            id("testing"),
            {},
            ast::query::table_value_constructor {
                    ast::scalar::value_constructor {
                            literal(number("1")),
                            literal(string("'a'")),
                            literal(string("'b'")),
                            literal(string("'c'")),
                    }
            },
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::write);
    auto&& stmt = downcast<tstatement::write>(**alternative);

    EXPECT_EQ(stmt.operator_kind(), tstatement::write_kind::insert);
    EXPECT_EQ(&extract<::yugawara::storage::index>(stmt.destination()).table(), table.get());

    auto&& columns = stmt.columns();
    ASSERT_EQ(columns.size(), 4);
    {
        EXPECT_EQ(&extract<::yugawara::storage::column>(columns[0]), &table->columns()[0]);
        EXPECT_EQ(&extract<::yugawara::storage::column>(columns[1]), &table->columns()[1]);
        EXPECT_EQ(&extract<::yugawara::storage::column>(columns[2]), &table->columns()[2]);
        EXPECT_EQ(&extract<::yugawara::storage::column>(columns[3]), &table->columns()[3]);
    }

    auto&& tuples = stmt.tuples();
    ASSERT_EQ(tuples.size(), 1);
    {
        auto&& row = tuples[0];
        EXPECT_EQ(row.elements()[0], immediate(1));
        EXPECT_EQ(row.elements()[1], immediate("a"));
        EXPECT_EQ(row.elements()[2], immediate("b"));
        EXPECT_EQ(row.elements()[3], immediate("c"));
    }
}

TEST_F(analyze_statement_dml_test, insert_statement_null) {
    options_.prefer_write_statement() = false;
    auto table = install_table("testing");

    auto r = analyze_statement(context(), ast::statement::insert_statement {
            id("testing"),
            {
                    id("k"),
                    id("v"),
            },
            ast::query::table_value_constructor {
                    ast::scalar::value_constructor {
                            literal(number("1")),
                            literal(ast::literal::null {}),
                    }
            },
    });
    auto alternative = std::get_if<execution_plan_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    auto&& graph = **alternative;
    ASSERT_EQ(graph.size(), 2);

    auto first = find_first<trelation::values>(graph);
    ASSERT_TRUE(first);
    ASSERT_EQ(first->columns().size(), 2);
    {
        auto&& row = first->rows()[0].elements();
        EXPECT_EQ(row[0], immediate(1));
        EXPECT_EQ(row[1], (tscalar::immediate {
                tvalue::unknown {},
                ttype::character { ttype::varying },
        }));
    }

    auto last = find_last<trelation::write>(graph);
    ASSERT_TRUE(last);
}

TEST_F(analyze_statement_dml_test, insert_statement_or_ignore) {
    options_.prefer_write_statement() = false;
    auto table = install_table("testing");

    auto r = analyze_statement(context(), ast::statement::insert_statement {
            id("testing"),
            {
                    id("k"),
            },
            ast::query::table_value_constructor {
                    ast::scalar::value_constructor {
                            literal(number("1")),
                    }
            },
            {
                    ast::statement::insert_statement_option::or_ignore,
            },
    });
    auto alternative = std::get_if<execution_plan_result_type >(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    auto&& graph = **alternative;
    ASSERT_EQ(graph.size(), 2);

    auto last = find_last<trelation::write>(graph);
    ASSERT_TRUE(last);
    EXPECT_EQ(last->operator_kind(), tstatement::write_kind::insert_skip);
}

TEST_F(analyze_statement_dml_test, insert_statement_or_replace) {
    options_.prefer_write_statement() = false;
    auto table = install_table("testing");

    auto r = analyze_statement(context(), ast::statement::insert_statement {
            id("testing"),
            {
                    id("k"),
            },
            ast::query::table_value_constructor {
                    ast::scalar::value_constructor {
                            literal(number("1")),
                    }
            },
            {
                    ast::statement::insert_statement_option::or_replace,
            },
    });
    auto alternative = std::get_if<execution_plan_result_type >(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    auto&& graph = **alternative;
    ASSERT_EQ(graph.size(), 2);

    auto last = find_last<trelation::write>(graph);
    ASSERT_TRUE(last);
    EXPECT_EQ(last->operator_kind(), tstatement::write_kind::insert_overwrite);
}

TEST_F(analyze_statement_dml_test, update_statement_simple) {
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

TEST_F(analyze_statement_dml_test, update_statement_modify_key) {
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

TEST_F(analyze_statement_dml_test, update_statement_multiple_set) {
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

TEST_F(analyze_statement_dml_test, update_statement_filter) {
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

TEST_F(analyze_statement_dml_test, delete_statement_simple) {
    auto table = install_table("testing");

    auto r = analyze_statement(context(), ast::statement::delete_statement {
            id("testing"),
            {},
    });
    auto alternative = std::get_if<execution_plan_result_type >(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    auto&& graph = **alternative;
    ASSERT_EQ(graph.size(), 2);

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

    auto last = find_last<trelation::write>(graph);
    ASSERT_TRUE(last);
    EXPECT_EQ(find_next(*first).get(), last.get());

    EXPECT_EQ(last->operator_kind(), tstatement::write_kind::delete_);
    EXPECT_EQ(&extract<::yugawara::storage::index>(last->destination()).table(), table.get());

    ASSERT_EQ(last->keys().size(), 1);
    {
        auto&& column = last->keys()[0];
        EXPECT_EQ(column.source(), variables[0]);
        EXPECT_EQ(&extract<::yugawara::storage::column>(column.destination()), &table->columns()[0]);
    }

    ASSERT_EQ(last->columns().size(), 0);
}

TEST_F(analyze_statement_dml_test, delete_statement_filter) {
    auto table = install_table("testing");

    auto r = analyze_statement(context(), ast::statement::delete_statement {
            id("testing"),
            literal(ast::literal::boolean { true }),
    });
    auto alternative = std::get_if<execution_plan_result_type >(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    auto&& graph = **alternative;
    ASSERT_EQ(graph.size(), 3);

    auto first = find_first<trelation::scan>(graph);
    ASSERT_TRUE(first);

    auto filter = find_next<trelation::filter>(*first);
    ASSERT_TRUE(filter);
    ASSERT_EQ(filter->condition(), immediate_bool(true));

    auto last = find_last<trelation::write>(graph);
    ASSERT_TRUE(last);
    EXPECT_EQ(find_next(*filter).get(), last.get());
}

} // namespace mizugaki::analyzer::details
