#include <mizugaki/parser/sql_parser.h>

#include <gtest/gtest.h>

#include <takatori/util/string_builder.h>

#include <mizugaki/ast/compilation_unit.h>

#include <mizugaki/ast/statement/empty_statement.h>
#include <mizugaki/ast/statement/select_statement.h>
#include <mizugaki/ast/statement/insert_statement.h>
#include <mizugaki/ast/statement/update_statement.h>
#include <mizugaki/ast/statement/delete_statement.h>

#include <mizugaki/ast/query/query.h>
#include <mizugaki/ast/query/table_value_constructor.h>
#include <mizugaki/ast/query/select_asterisk.h>
#include <mizugaki/ast/query/select_column.h>
#include <mizugaki/ast/query/grouping_element.h>
#include <mizugaki/ast/query/grouping_column.h>

#include <mizugaki/ast/table/table_reference.h>

#include <mizugaki/ast/scalar/literal_expression.h>
#include <mizugaki/ast/scalar/variable_reference.h>
#include <mizugaki/ast/scalar/field_reference.h>
#include <mizugaki/ast/scalar/comparison_predicate.h>
#include <mizugaki/ast/scalar/value_constructor.h>

#include <mizugaki/ast/name/simple.h>
#include <mizugaki/ast/literal/numeric.h>

namespace mizugaki::parser {

using namespace ast;

using ::takatori::util::object_creator;
using ::takatori::util::string_builder;

template<class T>
using node_ptr = ::takatori::util::unique_object_ptr<T>;

class sql_parser_test : public ::testing::Test {};

static statement::statement const& first(sql_parser::result_type const& result) {
    return *result.value()->statements().at(0);
}

static std::string diagnostics(sql_parser::result_type const& result) {
    if (auto&& d = result.diagnostic()) {
        return string_builder {}
                << d.message()
                << " : "
                << d.document()->contents(d.region().first(), d.region().size())
                << string_builder::to_string;
    }
    return {};
}

static scalar::literal_expression int_literal(std::string_view str) {
    return scalar::literal_expression {
            literal::numeric {
                    literal::kind::exact_numeric,
                    std::nullopt,
                    common::chars { str },
            },
    };
}

TEST_F(sql_parser_test, empty_document) {
    sql_parser parser;
    auto result = parser("-", "");
    ASSERT_TRUE(result);

    auto cu = std::move(result.value());
    EXPECT_EQ(*cu, (compilation_unit {}));
}

TEST_F(sql_parser_test, empty_statement) {
    sql_parser parser;
    auto result = parser("-", ";");
    ASSERT_TRUE(result);

    EXPECT_EQ(first(result), (statement::empty_statement {}));
}

TEST_F(sql_parser_test, select_asterisk) {
    sql_parser parser;
    auto result = parser("-", "SELECT * FROM T0;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(first(result), (statement::select_statement {
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

TEST_F(sql_parser_test, select_asterisk_qualified) {
    sql_parser parser;
    auto result = parser("-", "SELECT T0.* FROM T0;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(first(result), (statement::select_statement {
            query::query {
                    {},
                    { // SELECT
                            query::select_asterisk {
                                    scalar::variable_reference {
                                            name::simple { "T0" },
                                    },
                            },
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

TEST_F(sql_parser_test, select_column) {
    sql_parser parser;
    auto result = parser("-", "SELECT C0 FROM T0;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(first(result), (statement::select_statement {
            query::query {
                    {},
                    { // SELECT
                            query::select_column {
                                    scalar::variable_reference {
                                            name::simple { "C0" },
                                    },
                            },
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

TEST_F(sql_parser_test, select_column_name) {
    sql_parser parser;
    auto result = parser("-", "SELECT C0 AS x0 FROM T0;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(first(result), (statement::select_statement {
            query::query {
                    {},
                    { // SELECT
                            query::select_column {
                                    scalar::variable_reference {
                                            name::simple { "C0" },
                                    },
                                    name::simple { "x0" },
                            },
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

TEST_F(sql_parser_test, select_where) {
    sql_parser parser;
    auto result = parser("-", "SELECT * FROM T0 WHERE a < 0;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(first(result), (statement::select_statement {
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
                            scalar::comparison_predicate {
                                    scalar::variable_reference {
                                            name::simple { "a" },
                                    },
                                    scalar::comparison_operator::less_than,
                                    int_literal("0"),
                            },
                    }
            }
    }));
}

TEST_F(sql_parser_test, select_group_by_grand_total) {
    sql_parser parser;
    auto result = parser("-", "SELECT C0, C1 FROM T0 GROUP BY ();");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(first(result), (statement::select_statement {
            query::query {
                    {},
                    { // SELECT
                            query::select_column {
                                    scalar::variable_reference {
                                            name::simple { "C0" },
                                    },
                            },
                            query::select_column {
                                    scalar::variable_reference {
                                            name::simple { "C1" },
                                    },
                            },
                    },
                    { // FROM
                            table::table_reference {
                                    name::simple { "T0" },
                            },
                    },
                    { // WHERE
                    },
                    { // GROUP BY
                            {}, // grand total
                    },
            }
    }));
}

TEST_F(sql_parser_test, select_group_by_columns) {
    sql_parser parser;
    auto result = parser("-", "SELECT C0, C1 FROM T0 GROUP BY C0, C1;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(first(result), (statement::select_statement {
            query::query {
                    {},
                    { // SELECT
                            query::select_column {
                                    scalar::variable_reference {
                                            name::simple { "C0" },
                                    },
                            },
                            query::select_column {
                                    scalar::variable_reference {
                                            name::simple { "C1" },
                                    },
                            },
                    },
                    { // FROM
                            table::table_reference {
                                    name::simple { "T0" },
                            },
                    },
                    { // WHERE
                    },
                    { // GROUP BY
                            {
                                    query::grouping_column {
                                            scalar::variable_reference {
                                                    name::simple { "C0" },
                                            },
                                    },
                                    query::grouping_column {
                                            scalar::variable_reference {
                                                    name::simple { "C1" },
                                            },
                                    },
                            },
                    },
            }
    }));
}

TEST_F(sql_parser_test, select_group_by_having) {
    sql_parser parser;
    auto result = parser("-", "SELECT C0 FROM T0 GROUP BY C0, C1 HAVING C1 = 0;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(first(result), (statement::select_statement {
            query::query {
                    {},
                    { // SELECT
                            query::select_column {
                                    scalar::variable_reference {
                                            name::simple { "C0" },
                                    },
                            },
                    },
                    { // FROM
                            table::table_reference {
                                    name::simple { "T0" },
                            },
                    },
                    { // WHERE
                    },
                    { // GROUP BY
                            {
                                    query::grouping_column {
                                            scalar::variable_reference {
                                                    name::simple { "C0" },
                                            },
                                    },
                                    query::grouping_column {
                                            scalar::variable_reference {
                                                    name::simple { "C1" },
                                            },
                                    },
                            },
                    },
                    { // HAVING
                            scalar::comparison_predicate {
                                    scalar::variable_reference {
                                            name::simple { "C1" },
                                    },
                                    scalar::comparison_operator::equals,
                                    int_literal("0"),
                            },
                    },
            }
    }));
}

TEST_F(sql_parser_test, select_order_by) {
    sql_parser parser;
    auto result = parser("-", "SELECT C0 FROM T0 ORDER BY C0, C1 ASC, C2 DESC;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(first(result), (statement::select_statement {
            query::query {
                    {},
                    { // SELECT
                            query::select_column {
                                    scalar::variable_reference {
                                            name::simple { "C0" },
                                    },
                            },
                    },
                    { // FROM
                            table::table_reference {
                                    name::simple { "T0" },
                            },
                    },
                    { // WHERE
                    },
                    { // GROUP BY
                    },
                    { // HAVING
                    },
                    { // ORDER BY
                            name::simple { "C0" },
                            {
                                    name::simple { "C1" },
                                    common::ordering_specification::asc,
                            },
                            {
                                    name::simple { "C2" },
                                    common::ordering_specification::desc,
                            },
                    },
            }
    }));
}

TEST_F(sql_parser_test, select_limit) {
    sql_parser parser;
    auto result = parser("-", "SELECT C0 FROM T0 LIMIT 10;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(first(result), (statement::select_statement {
            query::query {
                    {},
                    { // SELECT
                            query::select_column {
                                    scalar::variable_reference {
                                            name::simple { "C0" },
                                    },
                            },
                    },
                    { // FROM
                            table::table_reference {
                                    name::simple { "T0" },
                            },
                    },
                    { // WHERE
                    },
                    { // GROUP BY
                    },
                    { // HAVING
                    },
                    { // ORDER BY
                    },
                    int_literal("10"),
            }
    }));
}

TEST_F(sql_parser_test, insert_select) {
    sql_parser parser;
    auto result = parser("-", "INSERT INTO T0 SELECT * FROM T1;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(first(result), (statement::insert_statement {
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

TEST_F(sql_parser_test, insert_values) {
    sql_parser parser;
    auto result = parser("-", "INSERT INTO T0 VALUES (1);");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(first(result), (statement::insert_statement {
            name::simple { "T0" },
            {},
            query::table_value_constructor {
                    scalar::value_constructor {
                            int_literal("1"),
                    },
            }
    }));
}

TEST_F(sql_parser_test, insert_values_multirow) {
    sql_parser parser;
    auto result = parser("-", "INSERT INTO T0 VALUES (1), (2), (3);");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(first(result), (statement::insert_statement {
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

TEST_F(sql_parser_test, insert_default_values) {
    sql_parser parser;
    auto result = parser("-", "INSERT INTO T0 DEFAULT VALUES;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(first(result), (statement::insert_statement {
            name::simple { "T0" },
            {},
            {},
    }));
}

TEST_F(sql_parser_test, insert_columns) {
    sql_parser parser;
    auto result = parser("-", "INSERT INTO T0 (C0, C1) VALUES (1, 2);");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(first(result), (statement::insert_statement {
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

TEST_F(sql_parser_test, update) {
    sql_parser parser;
    auto result = parser("-", "UPDATE T0 SET C0=1, C1 = 2;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(first(result), (statement::update_statement {
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

TEST_F(sql_parser_test, update_where) {
    sql_parser parser;
    auto result = parser("-", "UPDATE T0 SET C0=1, C1 = 2 WHERE C2 = 0;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(first(result), (statement::update_statement {
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

TEST_F(sql_parser_test, delete) {
    sql_parser parser;
    auto result = parser("-", "DELETE FROM T0;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(first(result), (statement::delete_statement {
            name::simple { "T0" },
    }));
}

TEST_F(sql_parser_test, delete_where) {
    sql_parser parser;
    auto result = parser("-", "DELETE FROM T0 WHERE C0 = 0;");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(first(result), (statement::delete_statement {
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

} // namespace mizugaki::parser
