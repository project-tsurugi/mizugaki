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

class sql_parser_statement_test : public ::testing::Test {
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

static std::string source(sql_parser::result_type const& result, ast::node const& element) {
    auto region = element.region();
    if (!region) {
        return {};
    }
    std::string text { result.value()->document()->contents(region.begin, region.size()) };
    return text;
}

static std::string source(sql_parser::result_type const& result) {
    return source(result, extract(result));
}

template<class T, class U>
static std::string description(sql_parser::result_type const& result, U const& element) {
    auto&& elem = downcast<T>(element);
    auto region = elem.description();
    if (!region) {
        return {};
    }
    std::string description { result.value()->document()->contents(region.begin, region.size()) };
    return description;
}

template<class T>
static std::string description(sql_parser::result_type const& result) {
    auto&& statement = extract(result);
    return description<T>(result, statement);
}

static statement::column_definition const& extract_column(sql_parser::result_type const& result, std::size_t column_position) {
    auto&& table = extract<statement::table_definition>(result);
    auto&& column = table.elements().at(column_position);
    return downcast<statement::column_definition>(*column);
}

TEST_F(sql_parser_statement_test, empty_document) {
    auto result = parse("");
    ASSERT_TRUE(result);

    auto cu = std::move(result.value());
    EXPECT_EQ(*cu, (compilation_unit {}));
}

TEST_F(sql_parser_statement_test, empty_statement) {
    auto result = parse(";");
    ASSERT_TRUE(result);

    EXPECT_EQ(extract(result), (statement::empty_statement {}));
}

TEST_F(sql_parser_statement_test, select_statement) {
    auto result = parse("SELECT * FROM T0");
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
    auto result = parse("INSERT INTO T0 SELECT * FROM T1;");
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
    auto result = parse("INSERT INTO T0 VALUES (1);");
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
    auto result = parse("INSERT INTO T0 VALUES (1), (2), (3);");
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
    auto result = parse("INSERT INTO T0 DEFAULT VALUES;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::insert_statement {
            name::simple { "T0" },
            {},
            {},
    }));
}

TEST_F(sql_parser_statement_test, insert_columns) {
    auto result = parse("INSERT INTO T0 (C0, C1) VALUES (1, 2);");
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
    auto result = parse("INSERT OR REPLACE INTO T0 VALUES (1);");
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
    auto result = parse("INSERT OR IGNORE INTO T0 VALUES (1);");
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
    auto result = parse("UPDATE OR INSERT INTO T0 VALUES (1);");
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
    auto result = parse("INSERT IF NOT EXISTS INTO T0 VALUES (1);");
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
    auto result = parse("UPDATE T0 SET C0=1, C1 = 2;");
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
    auto result = parse("UPDATE T0 SET C0=1, C1 = 2 WHERE C2 = 0;");
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
    auto result = parse("UPDATE T0 SET C0=1, C1 = 2 WHERE CURRENT OF cur;");
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
    auto result = parse("DELETE FROM T0;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::delete_statement {
            name::simple { "T0" },
    }));
}

TEST_F(sql_parser_statement_test, delete_where) {
    auto result = parse("DELETE FROM T0 WHERE C0 = 0;");
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
    auto result = parse("DELETE FROM T0 WHERE CURRENT OF cur;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::delete_statement {
            name::simple { "T0" },
            scalar::current_of_cursor {
                    name::simple { "cur" },
            },
    }));
}

TEST_F(sql_parser_statement_test, table_definition) {
    auto result = parse("CREATE TABLE t (id DECIMAL)");
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
    EXPECT_EQ(description<statement::table_definition>(result), "");
}

TEST_F(sql_parser_statement_test, table_definition_columns) {
    auto result = parse("CREATE TABLE t (a DECIMAL, b DECIMAL, c DECIMAL)");
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
    auto&& column = extract_column(result, 0);
    EXPECT_EQ(description<statement::column_definition>(result, column), "");
}

TEST_F(sql_parser_statement_test, table_definition_check) {
    auto result = parse("CREATE TABLE t (CHECK(x))");
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
    auto result = parse("CREATE TABLE t (UNIQUE(x))");
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
    auto result = parse("CREATE TABLE t (UNIQUE(a, b, c))");
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
    auto result = parse("CREATE TABLE t (UNIQUE(a ASC))");
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
    auto result = parse("CREATE TABLE t (UNIQUE(a DESC))");
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
    auto result = parse("CREATE TABLE t (UNIQUE(a NULLS FIRST))");
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
    auto result = parse("CREATE TABLE t (UNIQUE(a NULLS LAST))");
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
    auto result = parse("CREATE TABLE t (UNIQUE(a) INCLUDE(b, c))");
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
    auto result = parse("CREATE TABLE t (PRIMARY KEY(a))");
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
    auto result = parse("CREATE TABLE t (FOREIGN KEY (a) REFERENCES b)");
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
    auto result = parse("CREATE TABLE t (FOREIGN KEY (a, b, c) REFERENCES t (x, y, z))");
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
    auto result = parse("CREATE TABLE t (FOREIGN KEY (a) REFERENCES t ON UPDATE CASCADE)");
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
    auto result = parse("CREATE TABLE t (FOREIGN KEY (a) REFERENCES t ON DELETE SET NULL)");
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
    auto result = parse("CREATE TABLE t (FOREIGN KEY (a) REFERENCES t ON UPDATE RESTRICT ON DELETE NO ACTION)");
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
    auto result = parse("CREATE TABLE t (FOREIGN KEY (a) REFERENCES t ON DELETE RESTRICT ON UPDATE NO ACTION)");
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
    auto result = parse("CREATE TABLE t (a DECIMAL NULL)");
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
    auto result = parse("CREATE TABLE t (a DECIMAL NOT NULL)");
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
    auto result = parse("CREATE TABLE t (a DECIMAL CHECK(x))");
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
    auto result = parse("CREATE TABLE t (a DECIMAL DEFAULT x)");
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
    auto result = parse("CREATE TABLE t (a BIGINT DEFAULT NEXT VALUE FOR x)");
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
    auto result = parse("CREATE TABLE t (a DECIMAL GENERATED ALWAYS AS (x))");
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
    auto result = parse("CREATE TABLE t (a DECIMAL GENERATED ALWAYS AS IDENTITY)");
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
    auto result = parse("CREATE TABLE t (a DECIMAL GENERATED BY DEFAULT AS IDENTITY)");
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
    auto result = parse("CREATE TABLE t (a DECIMAL UNIQUE)");
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
    auto result = parse("CREATE TABLE t (a DECIMAL PRIMARY KEY)");
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
    auto result = parse("CREATE TABLE t (a DECIMAL REFERENCES s (k))");
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
    auto result = parse("CREATE TEMPORARY TABLE t (id DECIMAL)");
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
    auto result = parse("CREATE TABLE IF NOT EXISTS t (id DECIMAL)");
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
    auto result = parse("CREATE TABLE t (id DECIMAL) WITH (u=v)");
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
    auto result = parse("CREATE TABLE t (id DECIMAL) WITH (u=v, p=q, x=y)");
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

TEST_F(sql_parser_statement_test, table_definition_description) {
    auto result = parse(R"(
        /** description */
        CREATE TABLE t (id DECIMAL)
    )");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(description<statement::table_definition>(result), "/** description */");
    EXPECT_EQ(source(result).find("CREATE TABLE", 0), 0);
}

TEST_F(sql_parser_statement_test, table_definition_column_description) {
    auto result = parse(R"(
        CREATE TABLE t (
            /** column description */
            id DECIMAL
        )
    )");
    ASSERT_TRUE(result) << diagnostics(result);

    auto&& column = extract_column(result, 0);
    EXPECT_EQ(description<statement::column_definition>(result, column), "/** column description */");
    EXPECT_EQ(source(result, column).find("id DECIMAL", 0), 0);
}

TEST_F(sql_parser_statement_test, index_definition) {
    auto result = parse("CREATE INDEX i ON t (a)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::index_definition {
            name::simple { "i" },
            name::simple { "t" },
            {
                    name::simple { "a" },
            },
    }));
    EXPECT_EQ(description<statement::index_definition>(result), "");
}

TEST_F(sql_parser_statement_test, index_definition_unique) {
    auto result = parse("CREATE UNIQUE INDEX i ON t (a)");
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
    auto result = parse("CREATE INDEX IF NOT EXISTS i ON t (a)");
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
    auto result = parse("CREATE INDEX ON t (a)");
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
    auto result = parse("CREATE INDEX i ON t (a, b, c)");
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
    auto result = parse("CREATE INDEX i ON t (a ASC)");
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
    auto result = parse("CREATE INDEX i ON t (a NULLS FIRST)");
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
    auto result = parse("CREATE INDEX i ON t (a) INCLUDE (b, c)");
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
    auto result = parse("CREATE INDEX i ON t (a) WHERE p");
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
    auto result = parse("CREATE INDEX i ON t (a) WITH (p=q)");
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

TEST_F(sql_parser_statement_test, index_definition_description) {
    auto result = parse(R"(
        /** description */
        CREATE INDEX i ON t (a)
    )");
    ASSERT_TRUE(result) << diagnostics(result);
    EXPECT_EQ(description<statement::index_definition>(result), "/** description */");
    EXPECT_EQ(source(result).find("CREATE INDEX", 0), 0);
}

TEST_F(sql_parser_statement_test, view_definition) {
    auto result = parse("CREATE VIEW v AS VALUES (1)");
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
    EXPECT_EQ(description<statement::view_definition>(result), "");
}

TEST_F(sql_parser_statement_test, view_definition_or_replace) {
    auto result = parse("CREATE OR REPLACE VIEW v AS VALUES (1)");
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
    auto result = parse("CREATE TEMPORARY VIEW v AS VALUES (1)");
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
    auto result = parse("CREATE RECURSIVE VIEW v AS VALUES (1)");
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
    auto result = parse("CREATE VIEW IF NOT EXISTS v AS VALUES (1)");
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
    auto result = parse("CREATE VIEW v (a) AS VALUES (1)");
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
    auto result = parse("CREATE VIEW v (a, b, c) AS VALUES (1, 2, 3)");
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
    auto result = parse("CREATE VIEW v WITH (u=v) AS VALUES (1)");
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
TEST_F(sql_parser_statement_test, view_definition_description) {
    auto result = parse(R"(
        /** description */
        CREATE VIEW v AS VALUES (1)
    )");
    ASSERT_TRUE(result) << diagnostics(result);
    EXPECT_EQ(description<statement::view_definition>(result), "/** description */");
    EXPECT_EQ(source(result).find("CREATE VIEW", 0), 0);
}

TEST_F(sql_parser_statement_test, sequence_definition) {
    auto result = parse("CREATE SEQUENCE s");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::sequence_definition {
            name::simple { "s" },
    }));
    EXPECT_EQ(description<statement::sequence_definition>(result), "");
}

TEST_F(sql_parser_statement_test, sequence_definition_temporary) {
    auto result = parse("CREATE TEMPORARY SEQUENCE s");
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
    auto result = parse("CREATE SEQUENCE IF NOT EXISTS s");
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
    auto result = parse("CREATE SEQUENCE s AS DECIMAL");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::sequence_definition {
            name::simple { "s" },
            type::decimal {},
    }));
}

TEST_F(sql_parser_statement_test, sequence_definition_type_dup) {
    auto result = parse("CREATE SEQUENCE s AS DECIMAL AS DECIMAL");
    ASSERT_FALSE(result) << diagnostics(result);
}

TEST_F(sql_parser_statement_test, sequence_definition_owner) {
    auto result = parse("CREATE SEQUENCE s OWNED BY a.b");
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
    auto result = parse("CREATE SEQUENCE s OWNED BY a OWNED BY b");
    ASSERT_FALSE(result) << diagnostics(result);
}

TEST_F(sql_parser_statement_test, sequence_definition_start) {
    auto result = parse("CREATE SEQUENCE s START WITH 1");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::sequence_definition {
            name::simple { "s" },
            {},
            int_literal("1"),
    }));
}

TEST_F(sql_parser_statement_test, sequence_definition_start_shorten) {
    auto result = parse("CREATE SEQUENCE s START 1");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::sequence_definition {
            name::simple { "s" },
            {},
            int_literal("1"),
    }));
}

TEST_F(sql_parser_statement_test, sequence_definition_start_dup) {
    auto result = parse("CREATE SEQUENCE s START WITH 1 START WITH 1");
    ASSERT_FALSE(result) << diagnostics(result);
}

TEST_F(sql_parser_statement_test, sequence_definition_increment) {
    auto result = parse("CREATE SEQUENCE s INCREMENT BY 1");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::sequence_definition {
            name::simple { "s" },
            {},
            {},
            int_literal("1"),
    }));
}

TEST_F(sql_parser_statement_test, sequence_definition_increment_shorten) {
    auto result = parse("CREATE SEQUENCE s INCREMENT 1");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::sequence_definition {
            name::simple { "s" },
            {},
            {},
            int_literal("1"),
    }));
}

TEST_F(sql_parser_statement_test, sequence_definition_increment_dup) {
    auto result = parse("CREATE SEQUENCE s INCREMENT BY 1 INCREMENT BY 2");
    ASSERT_FALSE(result) << diagnostics(result);
}

TEST_F(sql_parser_statement_test, sequence_definition_max) {
    auto result = parse("CREATE SEQUENCE s MAXVALUE 1");
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
    auto result = parse("CREATE SEQUENCE s MAXVALUE 1 MAXVALUE 2");
    ASSERT_FALSE(result) << diagnostics(result);
}

TEST_F(sql_parser_statement_test, sequence_definition_no_max) {
    auto result = parse("CREATE SEQUENCE s NO MAXVALUE");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::sequence_definition {
            name::simple { "s" },
    }));
}

TEST_F(sql_parser_statement_test, sequence_definition_max_conflict) {
    auto result = parse("CREATE SEQUENCE s MAXVALUE 1 NO MAXVALUE");
    ASSERT_FALSE(result) << diagnostics(result);
}

TEST_F(sql_parser_statement_test, sequence_definition_min) {
    auto result = parse("CREATE SEQUENCE s MINVALUE 1");
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
    auto result = parse("CREATE SEQUENCE s MINVALUE 1 MINVALUE 2");
    ASSERT_FALSE(result) << diagnostics(result);
}

TEST_F(sql_parser_statement_test, sequence_definition_no_min) {
    auto result = parse("CREATE SEQUENCE s NO MINVALUE");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::sequence_definition {
            name::simple { "s" },
    }));
}

TEST_F(sql_parser_statement_test, sequence_definition_min_conflict) {
    auto result = parse("CREATE SEQUENCE s NO MINVALUE MINVALUE 1");
    ASSERT_FALSE(result) << diagnostics(result);
}

TEST_F(sql_parser_statement_test, sequence_definition_cycle) {
    auto result = parse("CREATE SEQUENCE s CYCLE");
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
    auto result = parse("CREATE SEQUENCE s CYCLE CYCLE");
    ASSERT_FALSE(result) << diagnostics(result);
}

TEST_F(sql_parser_statement_test, sequence_definition_no_cycle) {
    auto result = parse("CREATE SEQUENCE s NO CYCLE");
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
    auto result = parse("CREATE SEQUENCE s CYCLE NO CYCLE");
    ASSERT_FALSE(result) << diagnostics(result);
}

TEST_F(sql_parser_statement_test, sequence_definition_options) {
    auto result = parse(
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

TEST_F(sql_parser_statement_test, sequence_definition_description) {
    auto result = parse(R"(
        /** description */
        CREATE SEQUENCE s
    )");
    ASSERT_TRUE(result) << diagnostics(result);
    EXPECT_EQ(description<statement::sequence_definition>(result), "/** description */");
    EXPECT_EQ(source(result).find("CREATE SEQUENCE", 0), 0);
}

TEST_F(sql_parser_statement_test, schema_definition) {
    auto result = parse("CREATE SCHEMA s");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::schema_definition {
            name::simple { "s" },
    }));
    EXPECT_EQ(description<statement::schema_definition>(result), "");
}

TEST_F(sql_parser_statement_test, schema_definition_if_not_exists) {
    auto result = parse("CREATE SCHEMA IF NOT EXISTS s");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::schema_definition {
            name::simple { "s" },
            {},
            {},
            { statement::schema_definition_option::if_not_exists },
    }));
}

TEST_F(sql_parser_statement_test, schema_definition_authorization) {
    auto result = parse("CREATE SCHEMA AUTHORIZATION a");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::schema_definition {
            {},
            name::simple { "a" },
    }));
}

TEST_F(sql_parser_statement_test, schema_definition_name_authorization) {
    auto result = parse("CREATE SCHEMA s AUTHORIZATION a");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::schema_definition {
            name::simple { "s" },
            name::simple { "a" },
    }));
}

TEST_F(sql_parser_statement_test, schema_definition_authorization_current_role) {
    auto result = parse("CREATE SCHEMA AUTHORIZATION current_role");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::schema_definition {
            {},
            name::simple { "current_role" },
    }));
}

TEST_F(sql_parser_statement_test, schema_definition_elements) {
    auto result = parse("CREATE SCHEMA s CREATE TABLE t (id INT)");
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
    auto result = parse(R"(
        CREATE SCHEMA s
            CREATE TABLE t (id INT)
            CREATE INDEX i ON t(id)
            CREATE VIEW v AS VALUES(1)
    )");
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

TEST_F(sql_parser_statement_test, schema_definition_description) {
    auto result = parse(R"(
        /** description */
        CREATE SCHEMA s
    )");
    ASSERT_TRUE(result) << diagnostics(result);
    EXPECT_EQ(description<statement::schema_definition>(result), "/** description */");
    EXPECT_EQ(source(result).find("CREATE SCHEMA", 0), 0);
}

TEST_F(sql_parser_statement_test, schema_definition_elements_multiple_description) {
    auto result = parse(R"(
        CREATE SCHEMA s
            /** table */
            CREATE TABLE t (id INT)
            /** index */
            CREATE INDEX i ON t(id)
            /** view */
            CREATE VIEW v AS VALUES(1)
    )");
    ASSERT_TRUE(result) << diagnostics(result);
    auto&& schema = extract<statement::schema_definition>(result);

    ASSERT_EQ(schema.elements().size(), 3);
    EXPECT_EQ(description<statement::table_definition>(result, *schema.elements()[0]), "/** table */");
    EXPECT_EQ(description<statement::index_definition>(result, *schema.elements()[1]), "/** index */");
    EXPECT_EQ(description<statement::view_definition>(result, *schema.elements()[2]), "/** view */");
    EXPECT_EQ(source(result, *schema.elements()[0]).find("CREATE TABLE", 0), 0);
    EXPECT_EQ(source(result, *schema.elements()[1]).find("CREATE INDEX", 0), 0);
    EXPECT_EQ(source(result, *schema.elements()[2]).find("CREATE VIEW", 0), 0);
}

TEST_F(sql_parser_statement_test, drop_statement) {
    auto result = parse("DROP TABLE T0");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::drop_statement {
            statement::kind::drop_table_statement,
            name::simple { "T0" },
    }));
}

TEST_F(sql_parser_statement_test, drop_statement_qualified) {
    auto result = parse("DROP TABLE a.b.T0");
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
    auto result = parse("DROP TABLE IF EXISTS T0");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::drop_statement {
            statement::kind::drop_table_statement,
            name::simple { "T0" },
            { statement::drop_statement_option::if_exists },
    }));
}

TEST_F(sql_parser_statement_test, drop_statement_behavior) {
    auto result = parse("DROP TABLE T0 CASCADE");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::drop_statement {
            statement::kind::drop_table_statement,
            name::simple { "T0" },
            { statement::drop_statement_option::cascade },
    }));
}

TEST_F(sql_parser_statement_test, drop_statement_index) {
    auto result = parse("DROP INDEX T0");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::drop_statement {
            statement::kind::drop_index_statement,
            name::simple { "T0" },
    }));
}

TEST_F(sql_parser_statement_test, drop_statement_view) {
    auto result = parse("DROP VIEW T0");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::drop_statement {
            statement::kind::drop_view_statement,
            name::simple { "T0" },
    }));
}

TEST_F(sql_parser_statement_test, drop_statement_sequence) {
    auto result = parse("DROP SEQUENCE T0");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::drop_statement {
            statement::kind::drop_sequence_statement,
            name::simple { "T0" },
    }));
}

TEST_F(sql_parser_statement_test, drop_statement_schema) {
    auto result = parse("DROP SCHEMA T0");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (statement::drop_statement {
            statement::kind::drop_schema_statement,
            name::simple { "T0" },
    }));
}

} // namespace mizugaki::parser
