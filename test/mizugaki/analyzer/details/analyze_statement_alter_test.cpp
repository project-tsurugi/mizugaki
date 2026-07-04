#include <mizugaki/analyzer/details/analyze_statement.h>

#include <gtest/gtest.h>

#include <takatori/statement/empty.h>
#include <takatori/statement/rename_table.h>
#include <takatori/statement/rename_index.h>
#include <takatori/statement/rename_column.h>

#include <yugawara/binding/extract.h>

#include <yugawara/storage/table.h>
#include <yugawara/storage/index.h>

#include <mizugaki/ast/statement/alter_table_statement.h>
#include <mizugaki/ast/statement/alter_index_statement.h>
#include <mizugaki/ast/statement/rename_table_action.h>
#include <mizugaki/ast/statement/rename_index_action.h>
#include <mizugaki/ast/statement/rename_column_action.h>

#include "test_parent.h"

namespace mizugaki::analyzer::details {

using namespace ::mizugaki::analyzer::testing;

using ::yugawara::binding::extract;

class analyze_statement_alter_test : public test_parent {
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

TEST_F(analyze_statement_alter_test, alter_table_rename_table_simple) {
    auto table = install_table("testing");
    auto r = analyze_statement(context(), ast::statement::alter_table_statement {
            id("testing"),
            ast::statement::rename_table_action {
                    id("replacement")
            },
            false,
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();

    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::rename_table);
    auto&& stmt = downcast<tstatement::rename_table>(**alternative);

    EXPECT_EQ(&extract(stmt.schema()), default_schema_.get());

    auto&& target = extract(stmt.target());
    EXPECT_EQ(target, *table);

    EXPECT_EQ("replacement", stmt.replacement());
}

TEST_F(analyze_statement_alter_test, alter_table_rename_table_not_found) {
    invalid(sql_analyzer_code::table_not_found, ast::statement::alter_table_statement {
            id("testing"),
            ast::statement::rename_table_action {
                    id("replacement")
            },
            false,
    });
}

TEST_F(analyze_statement_alter_test, alter_table_rename_table_not_found_if_exists) {
    auto r = analyze_statement(context(), ast::statement::alter_table_statement {
            id("testing"),
            ast::statement::rename_table_action {
                    id("replacement")
            },
            true,
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();

    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::empty);
}

TEST_F(analyze_statement_alter_test, alter_table_rename_table_conflict) {
    install_table("testing");
    install_table("conflict");
    invalid(sql_analyzer_code::table_already_exists, ast::statement::alter_table_statement {
            id("testing"),
            ast::statement::rename_table_action {
                    id("conflict")
            },
            false,
    });
}

TEST_F(analyze_statement_alter_test, alter_table_rename_table_itself) {
    auto table = install_table("testing");
    invalid(sql_analyzer_code::table_already_exists, ast::statement::alter_table_statement {
            id("testing"),
            ast::statement::rename_table_action {
                    id("testing")
            },
            false,
    });
}

TEST_F(analyze_statement_alter_test, alter_table_rename_index_simple) {
    auto table = install_table("testing");
    auto index = storages_->add_index(::yugawara::storage::index {
            table,
            "idx",
            {
                    {
                            table->columns()[1],
                            ::yugawara::storage::index::key::direction_type::ascendant,
                    },
            },
            {},
            {
                    ::yugawara::storage::index_feature::find,
                    ::yugawara::storage::index_feature::scan,
                    ::yugawara::storage::index_feature::unique,
            },
    });
    auto r = analyze_statement(context(), ast::statement::alter_index_statement {
            id("idx"),
            ast::statement::rename_index_action {
                    id("replacement")
            },
            false,
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();

    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::rename_index);
    auto&& stmt = downcast<tstatement::rename_index>(**alternative);

    EXPECT_EQ(&extract(stmt.schema()), default_schema_.get());

    auto&& target = extract<::yugawara::storage::index>(stmt.target());
    EXPECT_EQ(target, *index);

    EXPECT_EQ("replacement", stmt.replacement());
}

TEST_F(analyze_statement_alter_test, alter_index_rename_index_not_found) {
    invalid(sql_analyzer_code::index_not_found, ast::statement::alter_index_statement {
            id("testing"),
            ast::statement::rename_index_action {
                    id("replacement")
            },
            false,
    });
}

TEST_F(analyze_statement_alter_test, alter_index_rename_index_not_found_if_exists) {
    auto r = analyze_statement(context(), ast::statement::alter_index_statement {
            id("testing"),
            ast::statement::rename_index_action {
                    id("replacement")
            },
            true,
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();

    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::empty);
}

TEST_F(analyze_statement_alter_test, alter_index_rename_index_conflict) {
    auto table = install_table("testing");
    storages_->add_index(::yugawara::storage::index {
            table,
            "idx",
            {
                    {
                            table->columns()[1],
                            ::yugawara::storage::index::key::direction_type::ascendant,
                    },
            },
            {},
            {
                    ::yugawara::storage::index_feature::find,
                    ::yugawara::storage::index_feature::scan,
                    ::yugawara::storage::index_feature::unique,
            },
    });
    storages_->add_index(::yugawara::storage::index {
            table,
            "conflict",
            {
                    {
                            table->columns()[2],
                            ::yugawara::storage::index::key::direction_type::ascendant,
                    },
            },
            {},
            {
                    ::yugawara::storage::index_feature::find,
                    ::yugawara::storage::index_feature::scan,
                    ::yugawara::storage::index_feature::unique,
            },
    });
    invalid(sql_analyzer_code::index_already_exists, ast::statement::alter_index_statement {
            id("idx"),
            ast::statement::rename_index_action {
                    id("conflict")
            },
            false,
    });
}

TEST_F(analyze_statement_alter_test, alter_index_rename_index_itself) {
    auto table = install_table("testing");
    storages_->add_index(::yugawara::storage::index {
            table,
            "idx",
            {
                    {
                            table->columns()[1],
                            ::yugawara::storage::index::key::direction_type::ascendant,
                    },
            },
            {},
            {
                    ::yugawara::storage::index_feature::find,
                    ::yugawara::storage::index_feature::scan,
                    ::yugawara::storage::index_feature::unique,
            },
    });
    invalid(sql_analyzer_code::index_already_exists, ast::statement::alter_index_statement {
            id("idx"),
            ast::statement::rename_index_action {
                    id("idx")
            },
            false,
    });
}

TEST_F(analyze_statement_alter_test, alter_index_rename_index_primary_index) {
    auto table = install_table("testing");
    invalid(sql_analyzer_code::unsupported_feature, ast::statement::alter_index_statement {
            id("testing"),
            ast::statement::rename_index_action {
                    id("replacement")
            },
            false,
    });
}

TEST_F(analyze_statement_alter_test, alter_table_rename_column_simple) {
    auto table = install_table("testing");
    auto r = analyze_statement(context(), ast::statement::alter_table_statement {
            id("testing"),
            ast::statement::rename_column_action {
                    id("v"),
                    id("replacement"),
                    false,
            },
            false,
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();

    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::rename_column);
    auto&& stmt = downcast<tstatement::rename_column>(**alternative);

    EXPECT_EQ(extract(stmt.table()), *table);

    auto&& target = extract<::yugawara::storage::column>(stmt.target());
    EXPECT_EQ(target, table->columns()[1]);

    EXPECT_EQ("replacement", stmt.replacement());
}

TEST_F(analyze_statement_alter_test, alter_table_rename_column_table_not_found) {
    invalid(sql_analyzer_code::table_not_found, ast::statement::alter_table_statement {
            id("testing"),
            ast::statement::rename_column_action {
                    id("v"),
                    id("replacement"),
                    false,
            },
            false,
    });
}

TEST_F(analyze_statement_alter_test, alter_table_rename_column_table_not_found_if_exists) {
    auto r = analyze_statement(context(), ast::statement::alter_table_statement {
            id("testing"),
            ast::statement::rename_column_action {
                    id("v"),
                    id("replacement"),
                    false,
            },
            true,
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();

    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::empty);
}

TEST_F(analyze_statement_alter_test, alter_table_rename_column_column_not_found) {
    auto table = install_table("testing");
    invalid(sql_analyzer_code::column_not_found, ast::statement::alter_table_statement {
            id("testing"),
            ast::statement::rename_column_action {
                    id("missing"),
                    id("replacement"),
                    false,
            },
            false,
    });
}

TEST_F(analyze_statement_alter_test, alter_table_rename_column_column_not_found_if_exists) {
    auto table = install_table("testing");
    auto r = analyze_statement(context(), ast::statement::alter_table_statement {
            id("testing"),
            ast::statement::rename_column_action {
                    id("missing"),
                    id("replacement"),
                    true,
            },
            false,
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative) << diagnostics();
    expect_no_error();

    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::empty);
}

TEST_F(analyze_statement_alter_test, alter_table_rename_column_conflict) {
    auto table = install_table("testing");
    invalid(sql_analyzer_code::column_already_exists, ast::statement::alter_table_statement {
            id("testing"),
            ast::statement::rename_column_action {
                    id("v"),
                    id("w"), // conflict
                    false,
            },
            false,
    });
}

TEST_F(analyze_statement_alter_test, alter_table_rename_column_itself) {
    auto table = install_table("testing");
    invalid(sql_analyzer_code::column_already_exists, ast::statement::alter_table_statement {
            id("testing"),
            ast::statement::rename_column_action {
                    id("v"),
                    id("v"),
                    false,
            },
            false,
    });
}

} // namespace mizugaki::analyzer::details
