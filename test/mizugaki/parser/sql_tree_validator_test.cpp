#include <mizugaki/parser/sql_tree_validator.h>

#include <gtest/gtest.h>

#include <string_view>

#include <mizugaki/ast/compilation_unit.h>

#include <mizugaki/ast/statement/select_statement.h>

#include <mizugaki/ast/query/query.h>
#include <mizugaki/ast/query/select_asterisk.h>
#include <mizugaki/ast/query/select_column.h>

#include <mizugaki/ast/table/table_reference.h>

#include <mizugaki/ast/scalar/binary_expression.h>
#include <mizugaki/ast/scalar/variable_reference.h>

#include <mizugaki/ast/name/simple.h>

namespace mizugaki::parser {

using namespace testing;

class sql_tree_validator_test : public ::testing::Test {
public:
    std::unique_ptr<ast::scalar::expression> build_expression(std::size_t leafs) {
        std::unique_ptr<ast::scalar::expression> expr = std::make_unique<ast::scalar::variable_reference>(
                std::make_unique<ast::name::simple>("x"));
        for (std::size_t index = 1; index < leafs; ++index) {
            expr = std::make_unique<ast::scalar::binary_expression>(
                    std::move(expr),
                    ast::scalar::binary_operator::plus,
                    std::make_unique<ast::scalar::variable_reference>(
                            std::make_unique<ast::name::simple>("x")));
        }
        return expr;
    }
};

TEST_F(sql_tree_validator_test, disabled) {
    // SELECT * FROM t;
    ast::compilation_unit unit {
            ast::statement::select_statement {
                    ast::query::query {
                            {
                                    ast::query::select_asterisk {},
                            },
                            {
                                    ast::table::table_reference {
                                            ast::name::simple { "table" },
                                    },
                            },
                    },
            },
    };
    sql_tree_validator validator { 0, 0 };
    auto error = validator(unit);
    EXPECT_FALSE(error);

    EXPECT_EQ(validator.last_node_count(), 0);
    EXPECT_EQ(validator.last_max_depth(), 0);
}

TEST_F(sql_tree_validator_test, counting) {
    // SELECT * FROM t;
    ast::compilation_unit unit {
            ast::statement::select_statement {
                    ast::query::query {
                            {
                                    ast::query::select_asterisk {},
                            },
                            {
                                    ast::table::table_reference {
                                            ast::name::simple { "t" },
                                    },
                            },
                    },
            },
    };
    sql_tree_validator validator { 100, 100 };
    auto error = validator(unit);
    EXPECT_FALSE(error);

    EXPECT_NE(validator.last_node_count(), 0);
    EXPECT_NE(validator.last_max_depth(), 0);
}

TEST_F(sql_tree_validator_test, deep) {
    // SELECT x + x + x + ... + x FROM t;
    ast::compilation_unit unit {
            ast::statement::select_statement {
                    ast::query::query {
                            {
                                    ast::query::select_column { build_expression(1'000) },
                            },
                            {
                                    ast::table::table_reference {
                                            ast::name::simple { "t" },
                                    },
                            },
                    },
            },
    };
    sql_tree_validator validator { 10000, 10000 };
    auto error = validator(unit);
    EXPECT_FALSE(error);

    EXPECT_GT(validator.last_node_count(), 3000);
    EXPECT_GT(validator.last_max_depth(), 1000);
}

TEST_F(sql_tree_validator_test, detect_count) {
    // SELECT x + x + x + ... + x FROM t;
    ast::compilation_unit unit {
            ast::statement::select_statement {
                    ast::query::query {
                            {
                                    ast::query::select_column { build_expression(1'000) },
                            },
                            {
                                    ast::table::table_reference {
                                            ast::name::simple { "t" },
                                    },
                            },
                    },
            },
    };
    sql_tree_validator validator { 3'000, 10'000 };
    auto error = validator(unit);
    ASSERT_TRUE(error);
    EXPECT_EQ(error->code(), sql_parser_code::exceed_number_of_elements);
}

TEST_F(sql_tree_validator_test, detect_depth) {
    // SELECT x + x + x + ... + x FROM t;
    ast::compilation_unit unit {
            ast::statement::select_statement {
                    ast::query::query {
                            {
                                    ast::query::select_column { build_expression(1'000) },
                            },
                            {
                                    ast::table::table_reference {
                                            ast::name::simple { "t" },
                                    },
                            },
                    },
            },
    };
    sql_tree_validator validator { 10'000, 1'000 };
    auto error = validator(unit);
    ASSERT_TRUE(error);
    EXPECT_EQ(error->code(), sql_parser_code::exceed_number_of_elements);
}

} // namespace mizugaki::parser
