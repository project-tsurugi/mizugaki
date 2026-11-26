#include <mizugaki/parser/sql_parser.h>

#include <gtest/gtest.h>

#include <mizugaki/ast/compilation_unit.h>

#include <mizugaki/ast/statement/alter_table_statement.h>
#include <mizugaki/ast/statement/alter_index_statement.h>

#include <mizugaki/ast/statement/rename_table_action.h>
#include <mizugaki/ast/statement/rename_column_action.h>
#include <mizugaki/ast/statement/rename_index_action.h>
#include <mizugaki/ast/statement/drop_partition_action.h>

#include <mizugaki/ast/scalar/comparison_predicate.h>
#include <mizugaki/ast/scalar/literal_expression.h>
#include <mizugaki/ast/literal/datetime.h>

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

TEST_F(sql_parser_statement_alter_test, alter_table_statement_drop_partition) {
    auto result = parse("ALTER TABLE t DROP PARTITION (a = 1)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (ast::statement::alter_table_statement {
        name::simple { "t" },
        ast::statement::drop_partition_action {
            {
                ast::statement::partition_criteria {
                    {
                        scalar::comparison_predicate {
                            v("a"),
                            scalar::comparison_operator::equals,
                            int_literal("1"),
                        },
                    },
                },
            },
        },
        false,
    }));
}

TEST_F(sql_parser_statement_alter_test, alter_table_statement_drop_partition_multiple) {
    auto result = parse("ALTER TABLE t DROP PARTITION (a = 1, b >= DATE '2000-01-01'), PARTITION (a = 2)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (ast::statement::alter_table_statement {
        name::simple { "t" },
        ast::statement::drop_partition_action {
            {
                ast::statement::partition_criteria {
                    {
                        scalar::comparison_predicate {
                            v("a"),
                            scalar::comparison_operator::equals,
                            int_literal("1"),
                        },
                        scalar::comparison_predicate {
                            v("b"),
                            scalar::comparison_operator::greater_than_or_equals,
                            scalar::literal_expression {
                                literal::datetime {
                                    literal::kind::date,
                                    "'2000-01-01'",
                                },
                            },
                        },
                    },
                },
                ast::statement::partition_criteria {
                    {
                        scalar::comparison_predicate {
                            v("a"),
                            scalar::comparison_operator::equals,
                            int_literal("2"),
                        },
                    },
                },
            },
        },
        false,
    }));
}

TEST_F(sql_parser_statement_alter_test, alter_table_statement_drop_partition_literal_first) {
    auto result = parse("ALTER TABLE t DROP PARTITION (1 < a)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (ast::statement::alter_table_statement {
        name::simple { "t" },
        ast::statement::drop_partition_action {
            {
                ast::statement::partition_criteria {
                    {
                        scalar::comparison_predicate {
                            int_literal("1"),
                            scalar::comparison_operator::less_than,
                            v("a"),
                        },
                    },
                },
            },
        },
        false,
    }));
}

TEST_F(sql_parser_statement_alter_test, alter_table_statement_drop_partition_not_equals) {
    auto result = parse("ALTER TABLE t DROP PARTITION (a <> 1)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (ast::statement::alter_table_statement {
        name::simple { "t" },
        ast::statement::drop_partition_action {
            {
                ast::statement::partition_criteria {
                    {
                        scalar::comparison_predicate {
                            v("a"),
                            scalar::comparison_operator::not_equals,
                            int_literal("1"),
                        },
                    },
                },
            },
        },
        false,
    }));
}

TEST_F(sql_parser_statement_alter_test, alter_table_statement_drop_partition_contextual_keywords) {
    auto result = parse("ALTER TABLE partition DROP PARTITION (range = 1)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (ast::statement::alter_table_statement {
        name::simple { "partition" },
        ast::statement::drop_partition_action {
            {
                ast::statement::partition_criteria {
                    {
                        scalar::comparison_predicate {
                            v("range"),
                            scalar::comparison_operator::equals,
                            int_literal("1"),
                        },
                    },
                },
            },
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
