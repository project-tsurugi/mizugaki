#include <mizugaki/analyzer/details/analyze_statement.h>

#include <gtest/gtest.h>

#include <takatori/value/primitive.h>
#include <takatori/type/primitive.h>

#include <takatori/relation/filter.h>
#include <takatori/relation/values.h>
#include <takatori/relation/write.h>

#include <takatori/statement/empty.h>
#include <takatori/statement/write.h>

#include <yugawara/binding/extract.h>

#include <yugawara/storage/index.h>
#include <yugawara/storage/column.h>

#include <mizugaki/ast/literal/special.h>

#include <mizugaki/ast/scalar/value_constructor.h>

#include <mizugaki/ast/query/table_value_constructor.h>

#include <mizugaki/ast/statement/empty_statement.h>
#include <mizugaki/ast/statement/insert_statement.h>

#include "test_parent.h"

namespace mizugaki::analyzer::details {

using namespace ::mizugaki::analyzer::testing;

using ::yugawara::binding::extract;

class analyze_statement_insert_test : public test_parent {
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

TEST_F(analyze_statement_insert_test, simple) {
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
    expect_no_error();

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

TEST_F(analyze_statement_insert_test, columns) {
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
    expect_no_error();

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

TEST_F(analyze_statement_insert_test, as_write_statement) {
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
    expect_no_error();

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

TEST_F(analyze_statement_insert_test, default_values) {
    options_.prefer_write_statement() = false;
    auto table = install_table("testing");

    auto r = analyze_statement(context(), ast::statement::insert_statement {
            id("testing"),
            {},
            {},
    });
    auto alternative = std::get_if<execution_plan_result_type >(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();

    auto&& graph = **alternative;
    ASSERT_EQ(graph.size(), 2);

    auto first = find_first<trelation::values>(graph);
    ASSERT_TRUE(first);
    ASSERT_EQ(first->columns().size(), 0);
    ASSERT_EQ(first->rows().size(), 1);
    ASSERT_EQ(first->rows()[0].elements().size(), 0);

    auto last = find_last<trelation::write>(graph);
    ASSERT_TRUE(last);
    EXPECT_EQ(find_next(*first).get(), last.get());

    EXPECT_EQ(last->operator_kind(), tstatement::write_kind::insert);
    EXPECT_EQ(&extract<::yugawara::storage::index>(last->destination()).table(), table.get());

    ASSERT_EQ(last->keys().size(), 0);
    ASSERT_EQ(last->columns().size(), 0);
}

TEST_F(analyze_statement_insert_test, default_values_as_write_statement) {
    options_.prefer_write_statement() = true;
    auto table = install_table("testing");

    auto r = analyze_statement(context(), ast::statement::insert_statement {
            id("testing"),
            {},
            {},
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();

    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::write);
    auto&& stmt = downcast<tstatement::write>(**alternative);

    EXPECT_EQ(stmt.operator_kind(), tstatement::write_kind::insert);
    EXPECT_EQ(&extract<::yugawara::storage::index>(stmt.destination()).table(), table.get());

    auto&& columns = stmt.columns();
    ASSERT_EQ(columns.size(), 0);
    auto&& tuples = stmt.tuples();
    ASSERT_EQ(tuples.size(), 1);
    ASSERT_EQ(tuples[0].elements().size(), 0);
}

TEST_F(analyze_statement_insert_test, values_null) {
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
    expect_no_error();

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

TEST_F(analyze_statement_insert_test, insert_or_ignore) {
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
    expect_no_error();

    auto&& graph = **alternative;
    ASSERT_EQ(graph.size(), 2);

    auto last = find_last<trelation::write>(graph);
    ASSERT_TRUE(last);
    EXPECT_EQ(last->operator_kind(), tstatement::write_kind::insert_skip);
}

TEST_F(analyze_statement_insert_test, insert_or_replace) {
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
    expect_no_error();

    auto&& graph = **alternative;
    ASSERT_EQ(graph.size(), 2);

    auto last = find_last<trelation::write>(graph);
    ASSERT_TRUE(last);
    EXPECT_EQ(last->operator_kind(), tstatement::write_kind::insert_overwrite);
}

TEST_F(analyze_statement_insert_test, invalid_relation) {
    invalid(ast::statement::insert_statement {
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
}

TEST_F(analyze_statement_insert_test, column_not_found) {
    auto table = install_table("testing");
    invalid(sql_analyzer_code::column_not_found, ast::statement::insert_statement {
            id("testing"),
            {
                    id("k"),
                    id("v"),
                    id("MISSING"),
                    id("x"),
            },
            ast::query::table_value_constructor {
                    ast::scalar::value_constructor {
                            literal(number("1")),
                            literal(string("'a'")),
                            literal(string("'b'")),
                            literal(string("'c'")),
                    }
            },
    });
}

TEST_F(analyze_statement_insert_test, column_duplicated) {
    auto table = install_table("testing");
    invalid(sql_analyzer_code::column_already_exists, ast::statement::insert_statement {
            id("testing"),
            {
                    id("k"),
                    id("v"),
                    id("w"),
                    id("x"),
                    id("v"), // duplicated
            },
            ast::query::table_value_constructor {
                    ast::scalar::value_constructor {
                            literal(number("1")),
                            literal(string("'a'")),
                            literal(string("'b'")),
                            literal(string("'c'")),
                            literal(string("'d'")),
                    }
            },
    });
}

TEST_F(analyze_statement_insert_test, invalid_query) {
    auto table = install_table("testing");
    invalid(ast::statement::insert_statement {
            id("testing"),
            {
                    id("k"),
                    id("v"),
                    id("w"),
                    id("x"),
            },
            ast::query::table_value_constructor {
                    ast::scalar::value_constructor {
                            literal(number("1")),
                            literal(string("'a'")),
                            literal(string("'b'")),
                            vref(id("MISSING")),
                    }
            },
    });
}

TEST_F(analyze_statement_insert_test, inconsistent_query) {
    auto table = install_table("testing");
    invalid(sql_analyzer_code::inconsistent_columns, ast::statement::insert_statement {
            id("testing"),
            {
                    id("k"),
                    id("v"),
                    id("w"),
                    id("x"),
            },
            ast::query::table_value_constructor {
                    ast::scalar::value_constructor {
                            literal(number("1")),
                            literal(string("'a'")),
                            literal(string("'b'")),
                            literal(string("'c'")),
                            literal(string("'d'")),
                    }
            },
    });
}

} // namespace mizugaki::analyzer::details
