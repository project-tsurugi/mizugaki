#include <mizugaki/analyzer/details/analyze_statement.h>

#include <gtest/gtest.h>

#include <takatori/statement/empty.h>
#include <takatori/statement/drop_table.h>
#include <takatori/statement/drop_index.h>

#include <yugawara/binding/extract.h>

#include <yugawara/storage/table.h>
#include <yugawara/storage/index.h>

#include <mizugaki/ast/statement/drop_statement.h>

#include "test_parent.h"

namespace mizugaki::analyzer::details {

using namespace ::mizugaki::analyzer::testing;

using ::yugawara::binding::extract;

class analyze_statement_ddl_test : public test_parent {
protected:
    void invalid(ast::statement::statement const& stmt) {
        trelation::graph_type graph {};
        auto r = analyze_statement(context(), stmt);
        EXPECT_TRUE(std::holds_alternative<erroneous_result_type>(r)) << diagnostics();
        EXPECT_GT(count_error(), 0);
    }

    void invalid(sql_analyzer_code code, ast::statement::statement const& stmt) {
        invalid(stmt);
        EXPECT_TRUE(find_error(code)) << code << " <=> " << diagnostics();
    }
};

TEST_F(analyze_statement_ddl_test, drop_table_simple) {
    auto table = install_table("testing");
    auto r = analyze_statement(context(), ast::statement::drop_statement {
            ast::statement::kind::drop_table_statement,
            id("TESTING"),
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();

    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::drop_table);

    auto&& stmt = downcast<tstatement::drop_table>(**alternative);
    EXPECT_EQ(&extract(stmt.schema()), default_schema_.get());

    auto&& target = extract(stmt.target());
    EXPECT_EQ(&target, table.get());
}

TEST_F(analyze_statement_ddl_test, drop_table_not_found) {
    invalid(sql_analyzer_code::table_not_found, ast::statement::drop_statement {
            ast::statement::kind::drop_table_statement,
            id("MISSING"),
    });
}

TEST_F(analyze_statement_ddl_test, drop_table_not_found_if_exists) {
    auto r = analyze_statement(context(), ast::statement::drop_statement {
            ast::statement::kind::drop_table_statement,
            id("MISSING"),
            {
                    ast::statement::drop_statement_option::if_exists,
            },
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();

    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::empty);
}

TEST_F(analyze_statement_ddl_test, drop_table_restrict) {
    auto table = install_table("testing");
    auto r = analyze_statement(context(), ast::statement::drop_statement {
            ast::statement::kind::drop_table_statement,
            id("testing"),
            {
                    ast::statement::drop_statement_option::restrict,
            },
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();

    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::drop_table);

    auto&& stmt = downcast<tstatement::drop_table>(**alternative);
    EXPECT_EQ(&extract(stmt.schema()), default_schema_.get());

    auto&& target = extract(stmt.target());
    EXPECT_EQ(&target, table.get());
}

TEST_F(analyze_statement_ddl_test, drop_table_cascade) {
    invalid(sql_analyzer_code::unsupported_feature, ast::statement::drop_statement {
            ast::statement::kind::drop_table_statement,
            id("MISSING"),
            {
                    ast::statement::drop_statement_option::cascade,
            },
    });
}

TEST_F(analyze_statement_ddl_test, drop_index_simple) {
    auto table = install_table("testing");
    auto index = storages_->add_index(::yugawara::storage::index {
            table,
            "testing_index",
            {
                    table->columns()[1],
            }
    });

    auto r = analyze_statement(context(), ast::statement::drop_statement {
            ast::statement::kind::drop_index_statement,
            id("TESTING_INDEX"),
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();

    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::drop_index);

    auto&& stmt = downcast<tstatement::drop_index>(**alternative);
    EXPECT_EQ(&extract(stmt.schema()), default_schema_.get());

    auto&& target = extract<::yugawara::storage::index>(stmt.target());
    EXPECT_EQ(&target, index.get());
}

TEST_F(analyze_statement_ddl_test, drop_index_not_found) {
    invalid(sql_analyzer_code::index_not_found, ast::statement::drop_statement {
            ast::statement::kind::drop_index_statement,
            id("MISSING"),
    });
}

TEST_F(analyze_statement_ddl_test, drop_index_not_found_if_exists) {
    auto r = analyze_statement(context(), ast::statement::drop_statement {
            ast::statement::kind::drop_index_statement,
            id("testing_index"),
            {
                    ast::statement::drop_statement_option::if_exists,
            },
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();

    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::empty);
}

TEST_F(analyze_statement_ddl_test, drop_index_restrict) {
    auto table = install_table("testing");
    auto index = storages_->add_index(::yugawara::storage::index {
            table,
            "testing_index",
            {
                    table->columns()[1],
            }
    });

    auto r = analyze_statement(context(), ast::statement::drop_statement {
            ast::statement::kind::drop_index_statement,
            id("testing_index"),
            {
                    ast::statement::drop_statement_option::restrict,
            },
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();

    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::drop_index);

    auto&& stmt = downcast<tstatement::drop_index>(**alternative);
    EXPECT_EQ(&extract(stmt.schema()), default_schema_.get());

    auto&& target = extract<::yugawara::storage::index>(stmt.target());
    EXPECT_EQ(&target, index.get());
}

TEST_F(analyze_statement_ddl_test, drop_index_cascade) {
    auto table = install_table("testing");
    auto index = storages_->add_index(::yugawara::storage::index {
            table,
            "testing_index",
            {
                    table->columns()[1],
            }
    });
    invalid(sql_analyzer_code::unsupported_feature, ast::statement::drop_statement {
            ast::statement::kind::drop_index_statement,
            id("testing_index"),
            {
                    ast::statement::drop_statement_option::cascade,
            },
    });
}

TEST_F(analyze_statement_ddl_test, drop_statement_restrict_vs_cascade) {
    auto table = install_table("testing");
    invalid(sql_analyzer_code::malformed_syntax, ast::statement::drop_statement {
            ast::statement::kind::drop_table_statement,
            id("testing"),
            {
                    ast::statement::drop_statement_option::restrict,
                    ast::statement::drop_statement_option::cascade,
            },
    });
}

} // namespace mizugaki::analyzer::details
