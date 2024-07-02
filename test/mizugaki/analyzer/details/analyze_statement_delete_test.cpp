#include <mizugaki/analyzer/details/analyze_statement.h>

#include <gtest/gtest.h>

#include <takatori/scalar/compare.h>

#include <takatori/relation/emit.h>
#include <takatori/relation/filter.h>
#include <takatori/relation/scan.h>
#include <takatori/relation/write.h>

#include <takatori/statement/write.h>

#include <yugawara/binding/extract.h>

#include <yugawara/storage/index.h>
#include <yugawara/storage/column.h>

#include <mizugaki/ast/scalar/comparison_predicate.h>
#include <mizugaki/ast/scalar/builtin_set_function_invocation.h>

#include <mizugaki/ast/statement/empty_statement.h>
#include <mizugaki/ast/statement/delete_statement.h>

#include "test_parent.h"

namespace mizugaki::analyzer::details {

using namespace ::mizugaki::analyzer::testing;

using ::yugawara::binding::extract;

class analyze_statement_delete_test : public test_parent {
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

TEST_F(analyze_statement_delete_test, simple) {
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

TEST_F(analyze_statement_delete_test, where) {
    auto table = install_table("testing");

    // DELETE FROM testing WHERE k > 0
    auto r = analyze_statement(context(), ast::statement::delete_statement {
            id("testing"),
            ast::scalar::comparison_predicate {
                    vref(id("k")),
                    ast::scalar::comparison_operator::greater_than,
                    literal(number("0")),
            },
    });
    auto alternative = std::get_if<execution_plan_result_type >(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    auto&& graph = **alternative;
    ASSERT_EQ(graph.size(), 3);

    auto first = find_first<trelation::scan>(graph);
    ASSERT_TRUE(first);

    auto&& scan_columns = first->columns();
    ASSERT_EQ(scan_columns.size(), 4);

    auto filter = find_next<trelation::filter>(*first);
    ASSERT_TRUE(filter);
    ASSERT_EQ(filter->condition(), (tscalar::compare {
            tscalar::comparison_operator::greater,
            vref(scan_columns[0].destination()), // k
            immediate(0),
    }));

    auto last = find_last<trelation::write>(graph);
    ASSERT_TRUE(last);
    EXPECT_EQ(find_next(*filter).get(), last.get());
}

TEST_F(analyze_statement_delete_test, invalid_relation) {
    invalid(ast::statement::delete_statement {
            id("MISSING"),
            {},
    });
}

TEST_F(analyze_statement_delete_test, invalid_where) {
    auto table = install_table("testing");
    invalid(ast::statement::delete_statement {
            id("testing"),
            ast::scalar::comparison_predicate {
                    vref(id("MISSING")),
                    ast::scalar::comparison_operator::greater_than,
                    literal(number("0")),
            },
    });
}

TEST_F(analyze_statement_delete_test, invalid_where_aggregated) {
    auto table = install_table("testing");
    auto count = set_functions_->add(::yugawara::aggregate::declaration {
            ::yugawara::aggregate::declaration::minimum_builtin_function_id + 1,
            "count",
            ttype::int8 {},
            {},
            true,
    });
    invalid(ast::statement::delete_statement {
            id("testing"),
            ast::scalar::comparison_predicate {
                    ast::scalar::builtin_set_function_invocation {
                            ast::scalar::builtin_set_function_kind::count,
                            {},
                            {},
                    },
                    ast::scalar::comparison_operator::greater_than,
                    literal(number("0")),
            },
    });
}

} // namespace mizugaki::analyzer::details
