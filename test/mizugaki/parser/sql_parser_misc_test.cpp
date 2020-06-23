#include <mizugaki/parser/sql_parser.h>

#include <gtest/gtest.h>

#include <mizugaki/ast/statement/select_statement.h>

#include <mizugaki/ast/query/table_reference.h>

#include "utils.h"

namespace mizugaki::parser {

using namespace testing;

class sql_parser_misc_test : public ::testing::Test {};

static query::expression const& extract(sql_parser::result_type const& result) {
    auto&& stmt = *result.value()->statements().at(0);
    auto&& select = downcast<statement::select_statement>(stmt);
    return *select.expression();
}

TEST_F(sql_parser_misc_test, delimited_identifier) {
    sql_parser parser;
    auto result = parser("-", R"(TABLE "TABLE";)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (query::table_reference {
            name::simple { "TABLE" },
    }));
}

} // namespace mizugaki::parser
