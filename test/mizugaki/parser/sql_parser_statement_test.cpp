#include <mizugaki/parser/sql_parser.h>

#include <gtest/gtest.h>

#include <mizugaki/ast/compilation_unit.h>

#include <mizugaki/ast/statement/empty_statement.h>
#include <mizugaki/ast/statement/select_statement.h>
#include <mizugaki/ast/statement/insert_statement.h>
#include <mizugaki/ast/statement/update_statement.h>
#include <mizugaki/ast/statement/delete_statement.h>

#include <mizugaki/ast/query/query.h>
#include <mizugaki/ast/query/table_value_constructor.h>
#include <mizugaki/ast/query/select_asterisk.h>

#include <mizugaki/ast/table/table_reference.h>

#include <mizugaki/ast/scalar/literal_expression.h>
#include <mizugaki/ast/scalar/variable_reference.h>
#include <mizugaki/ast/scalar/comparison_predicate.h>
#include <mizugaki/ast/scalar/value_constructor.h>
#include <mizugaki/ast/scalar/current_of_cursor.h>

#include <mizugaki/ast/name/simple.h>

#include "utils.h"

namespace mizugaki::parser {

using namespace testing;

class sql_parser_statement_test : public ::testing::Test {};

static statement::statement const& extract(sql_parser::result_type const& result) {
    return *result.value()->statements().at(0);
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
    auto result = parser("-", "SELECT * FROM T0;");
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

} // namespace mizugaki::parser
