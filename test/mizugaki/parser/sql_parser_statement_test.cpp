#include <mizugaki/parser/sql_parser.h>

#include <gtest/gtest.h>

#include <mizugaki/ast/compilation_unit.h>

#include <mizugaki/ast/statement/empty_statement.h>
#include <mizugaki/ast/statement/select_statement.h>
#include <mizugaki/ast/statement/insert_statement.h>
#include <mizugaki/ast/statement/update_statement.h>
#include <mizugaki/ast/statement/delete_statement.h>
#include <mizugaki/ast/statement/table_definition.h>
#include <mizugaki/ast/statement/index_definition.h>
#include <mizugaki/ast/statement/view_definition.h>
#include <mizugaki/ast/statement/sequence_definition.h>
#include <mizugaki/ast/statement/schema_definition.h>
#include <mizugaki/ast/statement/drop_statement.h>
#include <mizugaki/ast/statement/column_definition.h>
#include <mizugaki/ast/statement/table_constraint_definition.h>
#include <mizugaki/ast/statement/simple_constraint.h>
#include <mizugaki/ast/statement/expression_constraint.h>
#include <mizugaki/ast/statement/key_constraint.h>
#include <mizugaki/ast/statement/referential_constraint.h>
#include <mizugaki/ast/statement/identity_constraint.h>

#include <mizugaki/ast/query/query.h>
#include <mizugaki/ast/query/table_value_constructor.h>
#include <mizugaki/ast/query/select_asterisk.h>

#include <mizugaki/ast/table/table_reference.h>

#include <mizugaki/ast/type/simple.h>
#include <mizugaki/ast/type/decimal.h>

#include <mizugaki/ast/scalar/literal_expression.h>
#include <mizugaki/ast/scalar/variable_reference.h>
#include <mizugaki/ast/scalar/comparison_predicate.h>
#include <mizugaki/ast/scalar/value_constructor.h>
#include <mizugaki/ast/scalar/current_of_cursor.h>
#include <mizugaki/ast/scalar/builtin_function_invocation.h>

#include <mizugaki/ast/name/simple.h>
#include <mizugaki/ast/name/qualified.h>

#include "utils.h"

namespace mizugaki::parser {

using namespace testing;

class sql_parser_statement_test : public ::testing::Test {};

static statement::statement const& extract(sql_parser::result_type const& result) {
    auto&& statements = result.value()->statements();
    if (statements.size() >= 2) {
        using ::takatori::util::string_builder;
        throw std::runtime_error(string_builder {}
                << "extra statements: " << *result.value()
                << string_builder::to_string);
    }
    return *statements.at(0);
}

TEST_F(sql_parser_statement_test, empty_document) {
    sql_parser parser;
    auto result = parser("-", "");
    ASSERT_TRUE(result);

    auto cu = std::move(result.value());
    EXPECT_EQ(*cu, (compilation_unit {}));
}

TEST_F(sql_parser_statement_test, empty_statement) {
    sql_parser parser;
    auto result = parser("-", ";");
    ASSERT_TRUE(result);

    EXPECT_EQ(extract(result), (statement::empty_statement {}));
}

TEST_F(sql_parser_statement_test, select_statement) {
    sql_parser parser;
    auto result = parser("-", "SELECT * FROM T0");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::select_statement {
            query::query {
                    {},
                    { // SELECT
                            query::select_asterisk {},
                    },
                    { // FROM
                            table::table_reference {
                                    name::simple { "T0" },
                            },
                    },
                    { // WHERE
                    }
            }
    }));
}

TEST_F(sql_parser_statement_test, insert_select) {
    sql_parser parser;
    auto result = parser("-", "INSERT INTO T0 SELECT * FROM T1;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::insert_statement {
            name::simple { "T0" },
            {},
            query::query {
                    {},
                    { // SELECT
                            query::select_asterisk {},
                    },
                    { // FROM
                            table::table_reference {
                                    name::simple { "T1" },
                            },
                    },
            }
    }));
}

TEST_F(sql_parser_statement_test, insert_values) {
    sql_parser parser;
    auto result = parser("-", "INSERT INTO T0 VALUES (1);");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::insert_statement {
            name::simple { "T0" },
            {},
            query::table_value_constructor {
                    scalar::value_constructor {
                            int_literal("1"),
                    },
            }
    }));
}

TEST_F(sql_parser_statement_test, insert_values_multirow) {
    sql_parser parser;
    auto result = parser("-", "INSERT INTO T0 VALUES (1), (2), (3);");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::insert_statement {
            name::simple { "T0" },
            {},
            query::table_value_constructor {
                    scalar::value_constructor {
                            int_literal("1"),
                    },
                    scalar::value_constructor {
                            int_literal("2"),
                    },
                    scalar::value_constructor {
                            int_literal("3"),
                    },
            }
    }));
}

TEST_F(sql_parser_statement_test, insert_default_values) {
    sql_parser parser;
    auto result = parser("-", "INSERT INTO T0 DEFAULT VALUES;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::insert_statement {
            name::simple { "T0" },
            {},
            {},
    }));
}

TEST_F(sql_parser_statement_test, insert_columns) {
    sql_parser parser;
    auto result = parser("-", "INSERT INTO T0 (C0, C1) VALUES (1, 2);");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::insert_statement {
            name::simple { "T0" },
            {
                    name::simple { "C0" },
                    name::simple { "C1" },
            },
            query::table_value_constructor {
                    scalar::value_constructor {
                            int_literal("1"),
                            int_literal("2"),
                    },
            }
    }));
}

TEST_F(sql_parser_statement_test, insert_or_replace) {
    sql_parser parser;
    auto result = parser("-", "INSERT OR REPLACE INTO T0 VALUES (1);");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::insert_statement {
            name::simple { "T0" },
            {},
            query::table_value_constructor {
                    scalar::value_constructor {
                            int_literal("1"),
                    },
            },
            {
                    statement::insert_statement_option::or_replace,
            },
    }));
}

TEST_F(sql_parser_statement_test, insert_or_ignore) {
    sql_parser parser;
    auto result = parser("-", "INSERT OR IGNORE INTO T0 VALUES (1);");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::insert_statement {
            name::simple { "T0" },
            {},
            query::table_value_constructor {
                    scalar::value_constructor {
                            int_literal("1"),
                    },
            },
            {
                    statement::insert_statement_option::or_ignore,
            },
    }));
}

TEST_F(sql_parser_statement_test, update_or_insert) {
    sql_parser parser;
    auto result = parser("-", "UPDATE OR INSERT INTO T0 VALUES (1);");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::insert_statement {
            name::simple { "T0" },
            {},
            query::table_value_constructor {
                    scalar::value_constructor {
                            int_literal("1"),
                    },
            },
            {
                    statement::insert_statement_option::or_replace,
            },
    }));
}

TEST_F(sql_parser_statement_test, insert_if_not_exists) {
    sql_parser parser;
    auto result = parser("-", "INSERT IF NOT EXISTS INTO T0 VALUES (1);");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::insert_statement {
            name::simple { "T0" },
            {},
            query::table_value_constructor {
                    scalar::value_constructor {
                            int_literal("1"),
                    },
            },
            {
                    statement::insert_statement_option::or_ignore,
            },
    }));
}

TEST_F(sql_parser_statement_test, update) {
    sql_parser parser;
    auto result = parser("-", "UPDATE T0 SET C0=1, C1 = 2;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::update_statement {
            name::simple { "T0" },
            {
                    {
                            name::simple { "C0" },
                            int_literal("1"),
                    },
                    {
                            name::simple { "C1" },
                            int_literal("2"),
                    },
            },
    }));
}

TEST_F(sql_parser_statement_test, update_where) {
    sql_parser parser;
    auto result = parser("-", "UPDATE T0 SET C0=1, C1 = 2 WHERE C2 = 0;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::update_statement {
            name::simple { "T0" },
            {
                    {
                            name::simple { "C0" },
                            int_literal("1"),
                    },
                    {
                            name::simple { "C1" },
                            int_literal("2"),
                    },
            },
            scalar::comparison_predicate {
                    scalar::variable_reference {
                            name::simple { "C2" },
                    },
                    scalar::comparison_operator::equals,
                    int_literal("0"),
            },
    }));
}

TEST_F(sql_parser_statement_test, update_where_current_of_cursor) {
    sql_parser parser;
    auto result = parser("-", "UPDATE T0 SET C0=1, C1 = 2 WHERE CURRENT OF cur;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::update_statement {
            name::simple { "T0" },
            {
                    {
                            name::simple { "C0" },
                            int_literal("1"),
                    },
                    {
                            name::simple { "C1" },
                            int_literal("2"),
                    },
            },
            scalar::current_of_cursor {
                    name::simple { "cur" },
            },
    }));
}

TEST_F(sql_parser_statement_test, delete) {
    sql_parser parser;
    auto result = parser("-", "DELETE FROM T0;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::delete_statement {
            name::simple { "T0" },
    }));
}

TEST_F(sql_parser_statement_test, delete_where) {
    sql_parser parser;
    auto result = parser("-", "DELETE FROM T0 WHERE C0 = 0;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::delete_statement {
            name::simple { "T0" },
            scalar::comparison_predicate {
                    scalar::variable_reference {
                            name::simple { "C0" },
                    },
                    scalar::comparison_operator::equals,
                    int_literal("0"),
            },
    }));
}

TEST_F(sql_parser_statement_test, delete_where_current_of_cursor) {
    sql_parser parser;
    auto result = parser("-", "DELETE FROM T0 WHERE CURRENT OF cur;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::delete_statement {
            name::simple { "T0" },
            scalar::current_of_cursor {
                    name::simple { "cur" },
            },
    }));
}

TEST_F(sql_parser_statement_test, table_definition) {
    sql_parser parser;
    auto result = parser("-", "CREATE TABLE t (id DECIMAL)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::table_definition {
            name::simple { "t" },
            {
                statement::column_definition {
                    name::simple { "id" },
                    type::decimal {},
                },
            },
    }));
}

TEST_F(sql_parser_statement_test, table_definition_columns) {
    sql_parser parser;
    auto result = parser("-", "CREATE TABLE t (a DECIMAL, b DECIMAL, c DECIMAL)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::table_definition {
            name::simple { "t" },
            {
                    statement::column_definition {
                            name::simple { "a" },
                            type::decimal {},
                    },
                    statement::column_definition {
                            name::simple { "b" },
                            type::decimal {},
                    },
                    statement::column_definition {
                            name::simple { "c" },
                            type::decimal {},
                    },
            },
    }));
}

TEST_F(sql_parser_statement_test, table_definition_check) {
    sql_parser parser;
    auto result = parser("-", "CREATE TABLE t (CHECK(x))");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::table_definition {
            name::simple { "t" },
            {
                    statement::table_constraint_definition {
                            statement::expression_constraint {
                                    statement::constraint_kind::check,
                                    v("x"),
                            }
                    },
            },
    }));
}

TEST_F(sql_parser_statement_test, table_definition_unique) {
    sql_parser parser;
    auto result = parser("-", "CREATE TABLE t (UNIQUE(x))");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::table_definition {
            name::simple { "t" },
            {
                    statement::table_constraint_definition {
                            statement::key_constraint {
                                    statement::constraint_kind::unique,
                                    {
                                            v("x"),
                                    },
                            }
                    },
            },
    }));
}

TEST_F(sql_parser_statement_test, table_definition_unique_columns) {
    sql_parser parser;
    auto result = parser("-", "CREATE TABLE t (UNIQUE(a, b, c))");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::table_definition {
            name::simple { "t" },
            {
                    statement::table_constraint_definition {
                            statement::key_constraint {
                                    statement::constraint_kind::unique,
                                    {
                                            v("a"),
                                            v("b"),
                                            v("c"),
                                    },
                            }
                    },
            },
    }));
}

TEST_F(sql_parser_statement_test, table_definition_unique_asc) {
    sql_parser parser;
    auto result = parser("-", "CREATE TABLE t (UNIQUE(a ASC))");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::table_definition {
            name::simple { "t" },
            {
                    statement::table_constraint_definition {
                            statement::key_constraint {
                                    statement::constraint_kind::unique,
                                    {
                                            {
                                                    v("a"),
                                                    common::ordering_specification::asc,
                                            },
                                    },
                            }
                    },
            },
    }));
}

TEST_F(sql_parser_statement_test, table_definition_unique_desc) {
    sql_parser parser;
    auto result = parser("-", "CREATE TABLE t (UNIQUE(a DESC))");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::table_definition {
            name::simple { "t" },
            {
                    statement::table_constraint_definition {
                            statement::key_constraint {
                                    statement::constraint_kind::unique,
                                    {
                                            {
                                                    v("a"),
                                                    common::ordering_specification::desc,
                                            },
                                    },
                            }
                    },
            },
    }));
}

TEST_F(sql_parser_statement_test, table_definition_unique_nulls_first) {
    sql_parser parser;
    auto result = parser("-", "CREATE TABLE t (UNIQUE(a NULLS FIRST))");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::table_definition {
            name::simple { "t" },
            {
                    statement::table_constraint_definition {
                            statement::key_constraint {
                                    statement::constraint_kind::unique,
                                    {
                                            {
                                                    v("a"),
                                                    {},
                                                    {},
                                                    common::null_ordering_specification::first,
                                            },
                                    },
                            }
                    },
            },
    }));
}

TEST_F(sql_parser_statement_test, table_definition_unique_nulls_last) {
    sql_parser parser;
    auto result = parser("-", "CREATE TABLE t (UNIQUE(a NULLS LAST))");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::table_definition {
            name::simple { "t" },
            {
                    statement::table_constraint_definition {
                            statement::key_constraint {
                                    statement::constraint_kind::unique,
                                    {
                                            {
                                                    v("a"),
                                                    {},
                                                    {},
                                                    common::null_ordering_specification::last,
                                            },
                                    },
                            }
                    },
            },
    }));
}

TEST_F(sql_parser_statement_test, table_definition_unique_include) {
    sql_parser parser;
    auto result = parser("-", "CREATE TABLE t (UNIQUE(a) INCLUDE(b, c))");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::table_definition {
            name::simple { "t" },
            {
                    statement::table_constraint_definition {
                            statement::key_constraint {
                                    statement::constraint_kind::unique,
                                    {
                                            v("a"),
                                    },
                                    {
                                            v("b"),
                                            v("c"),
                                    },
                            }
                    },
            },
    }));
}

TEST_F(sql_parser_statement_test, table_definition_primary_key) {
    sql_parser parser;
    auto result = parser("-", "CREATE TABLE t (PRIMARY KEY(a))");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::table_definition {
            name::simple { "t" },
            {
                    statement::table_constraint_definition {
                            statement::key_constraint {
                                    statement::constraint_kind::primary_key,
                                    {
                                            v("a"),
                                    },
                            }
                    },
            },
    }));
}

TEST_F(sql_parser_statement_test, table_definition_foreign_key) {
    sql_parser parser;
    auto result = parser("-", "CREATE TABLE t (FOREIGN KEY (a) REFERENCES b)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::table_definition {
            name::simple { "t" },
            {
                    statement::table_constraint_definition {
                            statement::referential_constraint {
                                    {
                                            name::simple {"a" },
                                    },
                                    name::simple { "b" },
                            }
                    },
            },
    }));
}

TEST_F(sql_parser_statement_test, table_definition_foreign_key_columns) {
    sql_parser parser;
    auto result = parser("-", "CREATE TABLE t (FOREIGN KEY (a, b, c) REFERENCES t (x, y, z))");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::table_definition {
            name::simple { "t" },
            {
                    statement::table_constraint_definition {
                            statement::referential_constraint {
                                    {
                                            name::simple { "a" },
                                            name::simple { "b" },
                                            name::simple { "c" },
                                    },
                                    name::simple { "t" },
                                    {
                                            name::simple { "x" },
                                            name::simple { "y" },
                                            name::simple { "z" },
                                    },
                            }
                    },
            },
    }));
}

TEST_F(sql_parser_statement_test, table_definition_foreign_key_on_update) {
    sql_parser parser;
    auto result = parser("-", "CREATE TABLE t (FOREIGN KEY (a) REFERENCES t ON UPDATE CASCADE)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::table_definition {
            name::simple { "t" },
            {
                    statement::table_constraint_definition {
                            statement::referential_constraint {
                                    {
                                            name::simple { "a" },
                                    },
                                    name::simple { "t" },
                                    {},
                                    { statement::referential_action::cascade },
                                    {},
                            }
                    },
            },
    }));
}

TEST_F(sql_parser_statement_test, table_definition_foreign_key_on_delete) {
    sql_parser parser;
    auto result = parser("-", "CREATE TABLE t (FOREIGN KEY (a) REFERENCES t ON DELETE SET NULL)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::table_definition {
            name::simple { "t" },
            {
                    statement::table_constraint_definition {
                            statement::referential_constraint {
                                    {
                                            name::simple { "a" },
                                    },
                                    name::simple { "t" },
                                    {},
                                    {},
                                    { statement::referential_action::set_null },
                            }
                    },
            },
    }));
}

TEST_F(sql_parser_statement_test, table_definition_foreign_key_on_update_delete) {
    sql_parser parser;
    auto result = parser("-", "CREATE TABLE t (FOREIGN KEY (a) REFERENCES t ON UPDATE RESTRICT ON DELETE NO ACTION)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::table_definition {
            name::simple { "t" },
            {
                    statement::table_constraint_definition {
                            statement::referential_constraint {
                                    {
                                            name::simple { "a" },
                                    },
                                    name::simple { "t" },
                                    {},
                                    { statement::referential_action::restrict },
                                    { statement::referential_action::no_action },
                            }
                    },
            },
    }));
}

TEST_F(sql_parser_statement_test, table_definition_foreign_key_on_delete_update) {
    sql_parser parser;
    auto result = parser("-", "CREATE TABLE t (FOREIGN KEY (a) REFERENCES t ON DELETE RESTRICT ON UPDATE NO ACTION)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::table_definition {
            name::simple { "t" },
            {
                    statement::table_constraint_definition {
                            statement::referential_constraint {
                                    {
                                            name::simple { "a" },
                                    },
                                    name::simple { "t" },
                                    {},
                                    { statement::referential_action::no_action },
                                    { statement::referential_action::restrict },
                            }
                    },
            },
    }));
}

TEST_F(sql_parser_statement_test, table_definition_column_null) {
    sql_parser parser;
    auto result = parser("-", "CREATE TABLE t (a DECIMAL NULL)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::table_definition {
            name::simple { "t" },
            {
                    statement::column_definition {
                            name::simple { "a" },
                            type::decimal {},
                            {
                                        statement::simple_constraint { statement::constraint_kind::null },
                                },
                    },
            },
    }));
}

TEST_F(sql_parser_statement_test, table_definition_column_not_null) {
    sql_parser parser;
    auto result = parser("-", "CREATE TABLE t (a DECIMAL NOT NULL)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::table_definition {
            name::simple { "t" },
            {
                    statement::column_definition {
                            name::simple { "a" },
                            type::decimal {},
                            {
                                    statement::simple_constraint { statement::constraint_kind::not_null },
                            },
                    },
            },
    }));
}

TEST_F(sql_parser_statement_test, table_definition_column_check) {
    sql_parser parser;
    auto result = parser("-", "CREATE TABLE t (a DECIMAL CHECK(x))");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::table_definition {
            name::simple { "t" },
            {
                    statement::column_definition {
                            name::simple { "a" },
                            type::decimal {},
                            {
                                    statement::expression_constraint {
                                            statement::constraint_kind::check,
                                            v("x"),
                                    },
                            },
                    },
            },
    }));
}

TEST_F(sql_parser_statement_test, table_definition_column_default_clause) {
    sql_parser parser;
    auto result = parser("-", "CREATE TABLE t (a DECIMAL DEFAULT x)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::table_definition {
            name::simple { "t" },
            {
                    statement::column_definition {
                            name::simple { "a" },
                            type::decimal {},
                            {
                                    statement::expression_constraint {
                                            statement::constraint_kind::default_clause,
                                            v("x"),
                                    },
                            },
                    },
            },
    }));
}

TEST_F(sql_parser_statement_test, table_definition_column_default_next_sequence) {
    sql_parser parser;
    auto result = parser("-", "CREATE TABLE t (a BIGINT DEFAULT NEXT VALUE FOR x)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::table_definition {
            name::simple { "t" },
            {
                    statement::column_definition {
                            name::simple { "a" },
                            type::simple { type::kind::big_integer },
                            {
                                    statement::expression_constraint {
                                            statement::constraint_kind::default_clause,
                                            scalar::builtin_function_invocation {
                                                    scalar::builtin_function_kind::next_value_for,
                                                    {
                                                            v("x"),
                                                    },
                                            }
                                    },
                            },
                    },
            },
    }));
}

TEST_F(sql_parser_statement_test, table_definition_column_generation) {
    sql_parser parser;
    auto result = parser("-", "CREATE TABLE t (a DECIMAL GENERATED ALWAYS AS (x))");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::table_definition {
            name::simple { "t" },
            {
                    statement::column_definition {
                            name::simple { "a" },
                            type::decimal {},
                            {
                                    statement::expression_constraint {
                                                statement::constraint_kind::generation_clause,
                                                v("x"),
                                    },
                            },
                    },
            },
    }));
}

TEST_F(sql_parser_statement_test, table_definition_column_identity_always) {
    sql_parser parser;
    auto result = parser("-", "CREATE TABLE t (a DECIMAL GENERATED ALWAYS AS IDENTITY)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::table_definition {
            name::simple { "t" },
            {
                    statement::column_definition {
                            name::simple { "a" },
                            type::decimal {},
                            {
                                    statement::identity_constraint {
                                            statement::identity_generation_type::always,
                                    },
                            },
                    },
            },
    }));
}

TEST_F(sql_parser_statement_test, table_definition_column_identity_by_default) {
    sql_parser parser;
    auto result = parser("-", "CREATE TABLE t (a DECIMAL GENERATED BY DEFAULT AS IDENTITY)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::table_definition {
            name::simple { "t" },
            {
                    statement::column_definition {
                            name::simple { "a" },
                            type::decimal {},
                            {
                                    statement::identity_constraint {
                                            statement::identity_generation_type::by_default,
                                    },
                            },
                    },
            },
    }));
}

TEST_F(sql_parser_statement_test, table_definition_column_unique) {
    sql_parser parser;
    auto result = parser("-", "CREATE TABLE t (a DECIMAL UNIQUE)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::table_definition {
            name::simple { "t" },
            {
                    statement::column_definition {
                            name::simple { "a" },
                            type::decimal {},
                            {
                                    statement::key_constraint {
                                            statement::constraint_kind::unique,
                                    },
                            },
                    },
            },
    }));
}

TEST_F(sql_parser_statement_test, table_definition_column_primary_key) {
    sql_parser parser;
    auto result = parser("-", "CREATE TABLE t (a DECIMAL PRIMARY KEY)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::table_definition {
            name::simple { "t" },
            {
                    statement::column_definition {
                            name::simple { "a" },
                            type::decimal {},
                            {
                                    statement::key_constraint {
                                            statement::constraint_kind::primary_key,
                                    },
                            },
                    },
            },
    }));
}

TEST_F(sql_parser_statement_test, table_definition_column_referential) {
    sql_parser parser;
    auto result = parser("-", "CREATE TABLE t (a DECIMAL REFERENCES s (k))");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::table_definition {
            name::simple { "t" },
            {
                    statement::column_definition {
                            name::simple { "a" },
                            type::decimal {},
                            {
                                    statement::referential_constraint {
                                            {},
                                            name::simple { "s" },
                                            {
                                                    name::simple { "k" },
                                            },
                                    },
                            },
                    },
            },
    }));
}

TEST_F(sql_parser_statement_test, table_definition_temporary) {
    sql_parser parser;
    auto result = parser("-", "CREATE TEMPORARY TABLE t (id DECIMAL)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::table_definition {
            name::simple { "t" },
            {
                    statement::column_definition {
                            name::simple { "id" },
                            type::decimal {},
                    },
            },
            {
                    statement::table_definition_option::temporary,
            },
    }));
}

TEST_F(sql_parser_statement_test, table_definition_if_not_exists) {
    sql_parser parser;
    auto result = parser("-", "CREATE TABLE IF NOT EXISTS t (id DECIMAL)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::table_definition {
            name::simple { "t" },
            {
                    statement::column_definition {
                            name::simple { "id" },
                            type::decimal {},
                    },
            },
            {
                    statement::table_definition_option::if_not_exists,
            },
    }));
}

TEST_F(sql_parser_statement_test, table_definition_parameters) {
    sql_parser parser;
    auto result = parser("-", "CREATE TABLE t (id DECIMAL) WITH (u=v)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::table_definition {
            name::simple { "t" },
            {
                    statement::column_definition {
                            name::simple { "id" },
                            type::decimal {},
                    },
            },
            {},
            {
                    {
                            name::simple { "u" },
                            v("v"),
                    },
            },
    }));
}

TEST_F(sql_parser_statement_test, table_definition_parameters_multiple) {
    sql_parser parser;
    auto result = parser("-", "CREATE TABLE t (id DECIMAL) WITH (u=v, p=q, x=y)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::table_definition {
            name::simple { "t" },
            {
                    statement::column_definition {
                            name::simple { "id" },
                            type::decimal {},
                    },
            },
            {},
            {
                    {
                            name::simple { "u" },
                            v("v"),
                    },
                    {
                            name::simple { "p" },
                            v("q"),
                    },
                    {
                            name::simple { "x" },
                            v("y"),
                    },
            },
    }));
}

TEST_F(sql_parser_statement_test, index_definition) {
    sql_parser parser;
    auto result = parser("-", "CREATE INDEX i ON t (a)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::index_definition {
            name::simple { "i" },
            name::simple { "t" },
            {
                    name::simple { "a" },
            },
    }));
}

TEST_F(sql_parser_statement_test, index_definition_unique) {
    sql_parser parser;
    auto result = parser("-", "CREATE UNIQUE INDEX i ON t (a)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::index_definition {
            name::simple { "i" },
            name::simple { "t" },
            {
                    name::simple { "a" },
            },
            {},
            {},
            { statement::index_definition_option::unique },
    }));
}

TEST_F(sql_parser_statement_test, index_definition_if_not_exists) {
    sql_parser parser;
    auto result = parser("-", "CREATE INDEX IF NOT EXISTS i ON t (a)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::index_definition {
            name::simple { "i" },
            name::simple { "t" },
            {
                    name::simple { "a" },
            },
            {},
            {},
            { statement::index_definition_option::if_not_exists },
    }));
}

TEST_F(sql_parser_statement_test, index_definition_omit_name) {
    sql_parser parser;
    auto result = parser("-", "CREATE INDEX ON t (a)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::index_definition {
            {},
            name::simple { "t" },
            {
                    name::simple { "a" },
            },
    }));
}

TEST_F(sql_parser_statement_test, index_definition_columns) {
    sql_parser parser;
    auto result = parser("-", "CREATE INDEX i ON t (a, b, c)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::index_definition {
            name::simple { "i" },
            name::simple { "t" },
            {
                    name::simple { "a" },
                    name::simple { "b" },
                    name::simple { "c" },
            },
    }));
}

TEST_F(sql_parser_statement_test, index_definition_order) {
    sql_parser parser;
    auto result = parser("-", "CREATE INDEX i ON t (a ASC)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::index_definition {
            name::simple { "i" },
            name::simple { "t" },
            {
                    {
                            name::simple { "a" },
                            common::ordering_specification::asc,
                    },
            },
    }));
}
TEST_F(sql_parser_statement_test, index_definition_null_ordering) {
    sql_parser parser;
    auto result = parser("-", "CREATE INDEX i ON t (a NULLS FIRST)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::index_definition {
            name::simple { "i" },
            name::simple { "t" },
            {
                    {
                            name::simple { "a" },
                            {},
                            {},
                            common::null_ordering_specification::first,
                    },
            },
    }));
}

TEST_F(sql_parser_statement_test, index_definition_values) {
    sql_parser parser;
    auto result = parser("-", "CREATE INDEX i ON t (a) INCLUDE (b, c)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::index_definition {
            name::simple { "i" },
            name::simple { "t" },
            {
                    name::simple { "a" },
            },
            {
                    v("b"),
                    v("c"),
            }
    }));
}

TEST_F(sql_parser_statement_test, index_definition_predicate) {
    sql_parser parser;
    auto result = parser("-", "CREATE INDEX i ON t (a) WHERE p");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::index_definition {
            name::simple { "i" },
            name::simple { "t" },
            {
                    name::simple { "a" },
            },
            {},
            v("p"),

    }));
}

TEST_F(sql_parser_statement_test, index_definition_parameters) {
    sql_parser parser;
    auto result = parser("-", "CREATE INDEX i ON t (a) WITH (p=q)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::index_definition {
            name::simple { "i" },
            name::simple { "t" },
            {
                    name::simple { "a" },
            },
            {},
            {},
            {},
            {
                    {
                            name::simple { "p" },
                            v("q"),
                    }
            },
    }));
}

TEST_F(sql_parser_statement_test, view_definition) {
    sql_parser parser;
    auto result = parser("-", "CREATE VIEW v AS VALUES (1)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::view_definition {
            name::simple { "v" },
            {},
            query::table_value_constructor{
                    scalar::value_constructor{
                            int_literal("1"),
                    },
            },
    }));
}

TEST_F(sql_parser_statement_test, view_definition_or_replace) {
    sql_parser parser;
    auto result = parser("-", "CREATE OR REPLACE VIEW v AS VALUES (1)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::view_definition {
            name::simple { "v" },
            {},
            query::table_value_constructor{
                    scalar::value_constructor{
                            int_literal("1"),
                    },
            },
            {
                    statement::view_definition_option::or_replace,
            },
    }));
}


TEST_F(sql_parser_statement_test, view_definition_temporary) {
    sql_parser parser;
    auto result = parser("-", "CREATE TEMPORARY VIEW v AS VALUES (1)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::view_definition {
            name::simple { "v" },
            {},
            query::table_value_constructor{
                    scalar::value_constructor{
                            int_literal("1"),
                    },
            },
            {
                    statement::view_definition_option::temporary,
            }
    }));
}

TEST_F(sql_parser_statement_test, view_definition_recursive) {
    sql_parser parser;
    auto result = parser("-", "CREATE RECURSIVE VIEW v AS VALUES (1)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::view_definition {
            name::simple { "v" },
            {},
            query::table_value_constructor{
                    scalar::value_constructor{
                            int_literal("1"),
                    },
            },
            {
                    statement::view_definition_option::recursive,
            }
    }));
}

TEST_F(sql_parser_statement_test, view_definition_if_not_exists) {
    sql_parser parser;
    auto result = parser("-", "CREATE VIEW IF NOT EXISTS v AS VALUES (1)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::view_definition {
            name::simple { "v" },
            {},
            query::table_value_constructor{
                    scalar::value_constructor{
                            int_literal("1"),
                    },
            },
            {
                    statement::view_definition_option::if_not_exists,
            },
    }));
}

TEST_F(sql_parser_statement_test, view_definition_columns) {
    sql_parser parser;
    auto result = parser("-", "CREATE VIEW v (a) AS VALUES (1)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::view_definition {
            name::simple { "v" },
            {
                    name::simple { "a" },
            },
            query::table_value_constructor{
                    scalar::value_constructor{
                            int_literal("1"),
                    },
            },
    }));
}

TEST_F(sql_parser_statement_test, view_definition_columns_multiple) {
    sql_parser parser;
    auto result = parser("-", "CREATE VIEW v (a, b, c) AS VALUES (1, 2, 3)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::view_definition {
            name::simple { "v" },
            {
                    name::simple { "a" },
                    name::simple { "b" },
                    name::simple { "c" },
            },
            query::table_value_constructor{
                    scalar::value_constructor{
                            int_literal("1"),
                            int_literal("2"),
                            int_literal("3"),
                    },
            },
    }));
}

TEST_F(sql_parser_statement_test, view_definition_parameters) {
    sql_parser parser;
    auto result = parser("-", "CREATE VIEW v WITH (u=v) AS VALUES (1)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::view_definition {
            name::simple { "v" },
            {},
            query::table_value_constructor{
                    scalar::value_constructor{
                            int_literal("1"),
                    },
            },
            {},
            {
                    {
                            name::simple { "u" },
                            v("v"),
                    }
            }
    }));
}

TEST_F(sql_parser_statement_test, sequence_definition) {
    sql_parser parser;
    auto result = parser("-", "CREATE SEQUENCE s");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::sequence_definition {
            name::simple { "s" },
    }));
}

TEST_F(sql_parser_statement_test, sequence_definition_temporary) {
    sql_parser parser;
    auto result = parser("-", "CREATE TEMPORARY SEQUENCE s");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::sequence_definition {
            name::simple { "s" },
            {},
            {},
            {},
            {},
            {},
            {},
            {
                    statement::sequence_definition_option::temporary,
            },
    }));
}

TEST_F(sql_parser_statement_test, sequence_definition_if_not_exists) {
    sql_parser parser;
    auto result = parser("-", "CREATE SEQUENCE IF NOT EXISTS s");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::sequence_definition {
            name::simple { "s" },
            {},
            {},
            {},
            {},
            {},
            {},
            {
                    statement::sequence_definition_option::if_not_exists,
            },
    }));
}

TEST_F(sql_parser_statement_test, sequence_definition_type) {
    sql_parser parser;
    auto result = parser("-", "CREATE SEQUENCE s AS DECIMAL");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::sequence_definition {
            name::simple { "s" },
            type::decimal {},
    }));
}

TEST_F(sql_parser_statement_test, sequence_definition_type_dup) {
    sql_parser parser;
    auto result = parser("-", "CREATE SEQUENCE s AS DECIMAL AS DECIMAL");
    ASSERT_FALSE(result) << diagnostics(result);
}

TEST_F(sql_parser_statement_test, sequence_definition_owner) {
    sql_parser parser;
    auto result = parser("-", "CREATE SEQUENCE s OWNED BY a.b");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::sequence_definition {
            name::simple { "s" },
            {},
            {},
            {},
            {},
            {},
            name::qualified { name::simple { "a" }, name::simple { "b" } },
    }));
}

TEST_F(sql_parser_statement_test, sequence_definition_owner_dup) {
    sql_parser parser;
    auto result = parser("-", "CREATE SEQUENCE s OWNED BY a OWNED BY b");
    ASSERT_FALSE(result) << diagnostics(result);
}

TEST_F(sql_parser_statement_test, sequence_definition_start) {
    sql_parser parser;
    auto result = parser("-", "CREATE SEQUENCE s START WITH 1");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::sequence_definition {
            name::simple { "s" },
            {},
            int_literal("1"),
    }));
}

TEST_F(sql_parser_statement_test, sequence_definition_start_shorten) {
    sql_parser parser;
    auto result = parser("-", "CREATE SEQUENCE s START 1");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::sequence_definition {
            name::simple { "s" },
            {},
            int_literal("1"),
    }));
}

TEST_F(sql_parser_statement_test, sequence_definition_start_dup) {
    sql_parser parser;
    auto result = parser("-", "CREATE SEQUENCE s START WITH 1 START WITH 1");
    ASSERT_FALSE(result) << diagnostics(result);
}

TEST_F(sql_parser_statement_test, sequence_definition_increment) {
    sql_parser parser;
    auto result = parser("-", "CREATE SEQUENCE s INCREMENT BY 1");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::sequence_definition {
            name::simple { "s" },
            {},
            {},
            int_literal("1"),
    }));
}

TEST_F(sql_parser_statement_test, sequence_definition_increment_shorten) {
    sql_parser parser;
    auto result = parser("-", "CREATE SEQUENCE s INCREMENT 1");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::sequence_definition {
            name::simple { "s" },
            {},
            {},
            int_literal("1"),
    }));
}

TEST_F(sql_parser_statement_test, sequence_definition_increment_dup) {
    sql_parser parser;
    auto result = parser("-", "CREATE SEQUENCE s INCREMENT BY 1 INCREMENT BY 2");
    ASSERT_FALSE(result) << diagnostics(result);
}

TEST_F(sql_parser_statement_test, sequence_definition_max) {
    sql_parser parser;
    auto result = parser("-", "CREATE SEQUENCE s MAXVALUE 1");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::sequence_definition {
            name::simple { "s" },
            {},
            {},
            {},
            {},
            int_literal("1"),
    }));
}

TEST_F(sql_parser_statement_test, sequence_definition_max_dup) {
    sql_parser parser;
    auto result = parser("-", "CREATE SEQUENCE s MAXVALUE 1 MAXVALUE 2");
    ASSERT_FALSE(result) << diagnostics(result);
}

TEST_F(sql_parser_statement_test, sequence_definition_no_max) {
    sql_parser parser;
    auto result = parser("-", "CREATE SEQUENCE s NO MAXVALUE");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::sequence_definition {
            name::simple { "s" },
    }));
}

TEST_F(sql_parser_statement_test, sequence_definition_max_conflict) {
    sql_parser parser;
    auto result = parser("-", "CREATE SEQUENCE s MAXVALUE 1 NO MAXVALUE");
    ASSERT_FALSE(result) << diagnostics(result);
}

TEST_F(sql_parser_statement_test, sequence_definition_min) {
    sql_parser parser;
    auto result = parser("-", "CREATE SEQUENCE s MINVALUE 1");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::sequence_definition {
            name::simple { "s" },
            {},
            {},
            {},
            int_literal("1"),
    }));
}

TEST_F(sql_parser_statement_test, sequence_definition_min_dup) {
    sql_parser parser;
    auto result = parser("-", "CREATE SEQUENCE s MINVALUE 1 MINVALUE 2");
    ASSERT_FALSE(result) << diagnostics(result);
}

TEST_F(sql_parser_statement_test, sequence_definition_no_min) {
    sql_parser parser;
    auto result = parser("-", "CREATE SEQUENCE s NO MINVALUE");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::sequence_definition {
            name::simple { "s" },
    }));
}

TEST_F(sql_parser_statement_test, sequence_definition_min_conflict) {
    sql_parser parser;
    auto result = parser("-", "CREATE SEQUENCE s NO MINVALUE MINVALUE 1");
    ASSERT_FALSE(result) << diagnostics(result);
}

TEST_F(sql_parser_statement_test, sequence_definition_cycle) {
    sql_parser parser;
    auto result = parser("-", "CREATE SEQUENCE s CYCLE");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::sequence_definition {
            name::simple { "s" },
            {},
            {},
            {},
            {},
            {},
            {},
            {
                    statement::sequence_definition_option::cycle,
            },
    }));
}

TEST_F(sql_parser_statement_test, sequence_definition_cycle_dup) {
    sql_parser parser;
    auto result = parser("-", "CREATE SEQUENCE s CYCLE CYCLE");
    ASSERT_FALSE(result) << diagnostics(result);
}

TEST_F(sql_parser_statement_test, sequence_definition_no_cycle) {
    sql_parser parser;
    auto result = parser("-", "CREATE SEQUENCE s NO CYCLE");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::sequence_definition {
            name::simple { "s" },
            {},
            {},
            {},
            {},
            {},
            {},
            {
                    statement::sequence_definition_option::no_cycle,
            },
    }));
}

TEST_F(sql_parser_statement_test, sequence_definition_cycle_conflict) {
    sql_parser parser;
    auto result = parser("-", "CREATE SEQUENCE s CYCLE NO CYCLE");
    ASSERT_FALSE(result) << diagnostics(result);
}

TEST_F(sql_parser_statement_test, sequence_definition_options) {
    sql_parser parser;
    auto result = parser("-",
            "CREATE SEQUENCE s "
            "START WITH 1 "
            "INCREMENT BY 2 "
            "MINVALUE 3 "
            "MAXVALUE 4 "
            "CYCLE "
            "AS DECIMAL "
            "OWNED BY x");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::sequence_definition {
            name::simple { "s" },
            type::decimal {},
            int_literal("1"),
            int_literal("2"),
            int_literal("3"),
            int_literal("4"),
            name::simple { "x" },
            {
                    statement::sequence_definition_option::cycle,
            },
    }));
}

TEST_F(sql_parser_statement_test, schema_definition) {
    sql_parser parser;
    auto result = parser("-", "CREATE SCHEMA s");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::schema_definition {
            name::simple { "s" },
    }));
}

TEST_F(sql_parser_statement_test, schema_definition_if_not_exists) {
    sql_parser parser;
    auto result = parser("-", "CREATE SCHEMA IF NOT EXISTS s");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::schema_definition {
            name::simple { "s" },
            {},
            {},
            { statement::schema_definition_option::if_not_exists },
    }));
}

TEST_F(sql_parser_statement_test, schema_definition_authorization) {
    sql_parser parser;
    auto result = parser("-", "CREATE SCHEMA AUTHORIZATION a");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::schema_definition {
            {},
            name::simple { "a" },
    }));
}

TEST_F(sql_parser_statement_test, schema_definition_name_authorization) {
    sql_parser parser;
    auto result = parser("-", "CREATE SCHEMA s AUTHORIZATION a");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::schema_definition {
            name::simple { "s" },
            name::simple { "a" },
    }));
}

TEST_F(sql_parser_statement_test, schema_definition_authorization_current_role) {
    sql_parser parser;
    auto result = parser("-", "CREATE SCHEMA AUTHORIZATION current_role");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::schema_definition {
            {},
            name::simple { "current_role" },
    }));
}

TEST_F(sql_parser_statement_test, schema_definition_elements) {
    sql_parser parser;
    auto result = parser("-", "CREATE SCHEMA s CREATE TABLE t (id INT)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::schema_definition {
            name::simple { "s" },
            {},
            {
                    statement::table_definition {
                            name::simple { "t" },
                            {
                                    statement::column_definition {
                                            name::simple { "id" },
                                            type::simple { type::kind::integer },
                                    },
                            },
                    },
            },
    }));
}

TEST_F(sql_parser_statement_test, schema_definition_elements_multiple) {
    sql_parser parser;
    auto result = parser("-",
            "CREATE SCHEMA s "
            "  CREATE TABLE t (id INT) "
            "  CREATE INDEX i ON t(id) "
            "  CREATE VIEW v AS VALUES(1) ");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::schema_definition {
            name::simple { "s" },
            {},
            {
                    statement::table_definition {
                            name::simple { "t" },
                            {
                                    statement::column_definition {
                                            name::simple { "id" },
                                            type::simple { type::kind::integer },
                                    },
                            },
                    },
                    statement::index_definition {
                            name::simple { "i" },
                            name::simple { "t" },
                            {
                                    name::simple { "id" },
                            },
                    },
                    statement::view_definition {
                            name::simple { "v" },
                            {},
                            query::table_value_constructor{
                                    scalar::value_constructor{
                                            int_literal("1"),
                                    },
                            },
                    },
            },
    }));
}

TEST_F(sql_parser_statement_test, drop_statement) {
    sql_parser parser;
    auto result = parser("-", "DROP TABLE T0");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::drop_statement {
            statement::kind::drop_table_statement,
            name::simple { "T0" },
    }));
}

TEST_F(sql_parser_statement_test, drop_statement_qualified) {
    sql_parser parser;
    auto result = parser("-", "DROP TABLE a.b.T0");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::drop_statement {
            statement::kind::drop_table_statement,
            name::qualified {
                    name::simple { "a" },
                    name::simple { "b" },
                    name::simple { "T0" },
            },
    }));
}

TEST_F(sql_parser_statement_test, drop_statement_if_exists) {
    sql_parser parser;
    auto result = parser("-", "DROP TABLE IF EXISTS T0");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::drop_statement {
            statement::kind::drop_table_statement,
            name::simple { "T0" },
            { statement::drop_statement_option::if_exists },
    }));
}

TEST_F(sql_parser_statement_test, drop_statement_behavior) {
    sql_parser parser;
    auto result = parser("-", "DROP TABLE T0 CASCADE");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::drop_statement {
            statement::kind::drop_table_statement,
            name::simple { "T0" },
            { statement::drop_statement_option::cascade },
    }));
}

TEST_F(sql_parser_statement_test, drop_statement_index) {
    sql_parser parser;
    auto result = parser("-", "DROP INDEX T0");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::drop_statement {
            statement::kind::drop_index_statement,
            name::simple { "T0" },
    }));
}

TEST_F(sql_parser_statement_test, drop_statement_view) {
    sql_parser parser;
    auto result = parser("-", "DROP VIEW T0");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::drop_statement {
            statement::kind::drop_view_statement,
            name::simple { "T0" },
    }));
}

TEST_F(sql_parser_statement_test, drop_statement_sequence) {
    sql_parser parser;
    auto result = parser("-", "DROP SEQUENCE T0");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::drop_statement {
            statement::kind::drop_sequence_statement,
            name::simple { "T0" },
    }));
}

TEST_F(sql_parser_statement_test, drop_statement_schema) {
    sql_parser parser;
    auto result = parser("-", "DROP SCHEMA T0");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::drop_statement {
            statement::kind::drop_schema_statement,
            name::simple { "T0" },
    }));
}

} // namespace mizugaki::parser
