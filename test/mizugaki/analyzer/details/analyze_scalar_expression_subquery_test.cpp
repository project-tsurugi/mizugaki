#include <mizugaki/analyzer/details/analyze_scalar_expression.h>

#include <gtest/gtest.h>

#include <takatori/value/primitive.h>
#include <takatori/type/primitive.h>

#include <takatori/relation/scan.h>
#include <takatori/relation/values.h>

#include <yugawara/extension/scalar/subquery.h>

#include <yugawara/binding/extract.h>

#include <mizugaki/ast/literal/boolean.h>
#include <mizugaki/ast/literal/special.h>

#include <mizugaki/ast/scalar/value_constructor.h>
#include <mizugaki/ast/scalar/subquery.h>
#include <mizugaki/ast/query/query.h>
#include <mizugaki/ast/query/table_reference.h>
#include <mizugaki/ast/query/table_value_constructor.h>

#include "test_parent.h"

namespace mizugaki::analyzer::details {

using namespace ::mizugaki::analyzer::testing;

class analyze_scalar_expression_subquery_test : public test_parent {
protected:
    query_scope scope;

    void invalid(ast::scalar::expression const& expression) {
        auto r = analyze_scalar_expression(
                context(),
                expression,
                scope,
                {});
        EXPECT_FALSE(r) << diagnostics();
        EXPECT_NE(count_error(), 0);
    }

    void invalid(sql_analyzer_code code, ast::scalar::expression const& expression) {
        invalid(expression);
        EXPECT_TRUE(find_error(code));
    }

    ast::scalar::literal_expression erroneous_expression() {
        return literal(string("INVALID"));
    }
};

TEST_F(analyze_scalar_expression_subquery_test, simple) {
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::subquery {
                    ast::query::table_value_constructor {
                            ast::scalar::value_constructor {
                                    literal(number("1")),
                            },
                    },
            },
            scope,
            {});
    ASSERT_TRUE(r) << diagnostics();

    auto&& subquery = downcast<::yugawara::extension::scalar::subquery>(*r);
    auto&& subgraph = subquery.query_graph();
    ASSERT_EQ(subgraph.size(), 1);

    auto&& column = subquery.output_column();

    auto&& subquery_output = subquery.find_output_port();
    ASSERT_TRUE(subquery_output);
    ASSERT_FALSE(subquery_output->opposite());
    ASSERT_TRUE(subgraph.contains(subquery_output->owner()));

    // values -
    auto&& values = downcast<trelation::values>(subquery_output->owner());
    auto&& values_columns = values.columns();
    ASSERT_EQ(values_columns.size(), 1);
    EXPECT_EQ(values_columns[0], column);
}

TEST_F(analyze_scalar_expression_subquery_test, table) {
    auto table = install_table("t");
    ::yugawara::storage::column_feature_set system_flags {
            ::yugawara::storage::column_feature::synthesized,
            ::yugawara::storage::column_feature::hidden,
    };
    table->columns()[0].features() += system_flags;
    table->columns()[2].features() += system_flags;
    table->columns()[3].features() += system_flags;
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::subquery {
                    ast::query::table_reference {
                            id("t")
                    },
            },
            scope,
            {});
    ASSERT_TRUE(r) << diagnostics();

    auto&& subquery = downcast<::yugawara::extension::scalar::subquery>(*r);
    auto&& subgraph = subquery.query_graph();
    ASSERT_EQ(subgraph.size(), 1);

    auto&& column = subquery.output_column();

    auto&& subquery_output = subquery.find_output_port();
    ASSERT_TRUE(subquery_output);
    ASSERT_FALSE(subquery_output->opposite());
    ASSERT_TRUE(subgraph.contains(subquery_output->owner()));

    // scan -
    auto&& scan = downcast<trelation::scan>(subquery_output->owner());
    auto&& scan_columns = scan.columns();
    ASSERT_EQ(scan_columns.size(), 1);
    EXPECT_EQ(scan_columns[0].destination(), column);
}

TEST_F(analyze_scalar_expression_subquery_test, columns_empty) {
    invalid(sql_analyzer_code::inconsistent_columns, ast::scalar::subquery {
            ast::query::table_value_constructor {
                    ast::scalar::value_constructor {},
            },
    });
}

TEST_F(analyze_scalar_expression_subquery_test, columns_over) {
    invalid(sql_analyzer_code::inconsistent_columns, ast::scalar::subquery {
            ast::query::table_value_constructor {
                    ast::scalar::value_constructor {
                            literal(number("1")),
                            literal(number("2")), // extra column
                    },
            },
    });
}

TEST_F(analyze_scalar_expression_subquery_test, row_subquery_unsupported) {
    invalid(sql_analyzer_code::unsupported_feature, ast::scalar::subquery {
            ast::query::table_value_constructor {
                    ast::scalar::value_constructor {
                            literal(number("1")),
                    },
            },
            ast::scalar::expression_context_kind::row,
    });
}

} // namespace mizugaki::analyzer::details
