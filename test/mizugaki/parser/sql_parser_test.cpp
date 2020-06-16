#include <mizugaki/parser/sql_parser.h>

#include <gtest/gtest.h>

#include <mizugaki/ast/compilation_unit.h>

#include <mizugaki/ast/statement/empty_statement.h>

namespace mizugaki::parser {

class sql_parser_test : public ::testing::Test {};

using namespace ast;

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

    auto cu = std::move(result.value());
    EXPECT_EQ(*cu, (compilation_unit {
            statement::empty_statement {},
    }));
}

} // namespace mizugaki::parser
