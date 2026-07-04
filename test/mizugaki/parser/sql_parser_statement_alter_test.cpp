#include <mizugaki/parser/sql_parser.h>

#include <gtest/gtest.h>

#include <mizugaki/ast/compilation_unit.h>

#include <mizugaki/ast/statement/alter_table_statement.h>
#include <mizugaki/ast/statement/alter_index_statement.h>

#include <mizugaki/ast/statement/rename_table_action.h>
#include <mizugaki/ast/statement/rename_column_action.h>
#include <mizugaki/ast/statement/rename_index_action.h>

#include <mizugaki/ast/name/simple.h>

#include "utils.h"

namespace mizugaki::parser {

using namespace testing;

class sql_parser_statement_alter_test : public ::testing::Test {
protected:
    sql_parser_result parse(std::string_view str, sql_parser parser = default_parser()) {
        return parser("-", std::string { str });
    }
};

template<class T = statement::statement>
static T const& extract(sql_parser::result_type const& result) {
    auto&& statements = result.value()->statements();
    if (statements.size() >= 2) {
        using ::takatori::util::string_builder;
        throw std::runtime_error(string_builder {}
                << "extra statements: " << *result.value()
                << string_builder::to_string);
    }
    return downcast<T>(*statements.at(0));
}

TEST_F(sql_parser_statement_alter_test, alter_table_statement_rename_table) {
    auto result = parse("ALTER TABLE t RENAME TO x");
    ASSERT_TRUE(result);

    EXPECT_EQ(extract(result), (ast::statement::alter_table_statement {
        name::simple { "t" },
        ast::statement::rename_table_action {
            name::simple { "x" },
        },
        false,
    }));
}

TEST_F(sql_parser_statement_alter_test, alter_table_statement_rename_table_if_exists) {
    auto result = parse("ALTER TABLE IF EXISTS t RENAME TO x");
    ASSERT_TRUE(result);

    EXPECT_EQ(extract(result), (ast::statement::alter_table_statement {
        name::simple { "t" },
        ast::statement::rename_table_action {
            name::simple { "x" },
        },
        true,
    }));
}

TEST_F(sql_parser_statement_alter_test, alter_table_statement_rename_column) {
    auto result = parse("ALTER TABLE t RENAME COLUMN c TO x");
    ASSERT_TRUE(result);

    EXPECT_EQ(extract(result), (ast::statement::alter_table_statement {
        name::simple { "t" },
        ast::statement::rename_column_action {
            name::simple { "c" },
            name::simple { "x" },
            false,
        },
        false,
    }));
}

TEST_F(sql_parser_statement_alter_test, alter_table_statement_rename_column_if_exists_table) {
    auto result = parse("ALTER TABLE IF EXISTS t RENAME COLUMN c TO x");
    ASSERT_TRUE(result);

    EXPECT_EQ(extract(result), (ast::statement::alter_table_statement {
        name::simple { "t" },
        ast::statement::rename_column_action {
            name::simple { "c" },
            name::simple { "x" },
            false,
        },
        true,
    }));
}

TEST_F(sql_parser_statement_alter_test, alter_table_statement_rename_column_if_exists_column) {
    auto result = parse("ALTER TABLE t RENAME COLUMN IF EXISTS c TO x");
    ASSERT_TRUE(result);

    EXPECT_EQ(extract(result), (ast::statement::alter_table_statement {
        name::simple { "t" },
        ast::statement::rename_column_action {
            name::simple { "c" },
            name::simple { "x" },
            true,
        },
        false,
    }));
}

TEST_F(sql_parser_statement_alter_test, alter_index_statement_rename_index) {
    auto result = parse("ALTER INDEX t RENAME TO x");
    ASSERT_TRUE(result);

    EXPECT_EQ(extract(result), (ast::statement::alter_index_statement {
        name::simple { "t" },
        ast::statement::rename_index_action {
            name::simple { "x" },
        },
        false,
    }));
}

TEST_F(sql_parser_statement_alter_test, alter_index_statement_rename_index_if_exists) {
    auto result = parse("ALTER INDEX IF EXISTS t RENAME TO x");
    ASSERT_TRUE(result);

    EXPECT_EQ(extract(result), (ast::statement::alter_index_statement {
        name::simple { "t" },
        ast::statement::rename_index_action {
            name::simple { "x" },
        },
        true,
    }));
}

} // namespace mizugaki::parser
