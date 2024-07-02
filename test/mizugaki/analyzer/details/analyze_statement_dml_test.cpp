#include <mizugaki/analyzer/details/analyze_statement.h>

#include <gtest/gtest.h>

#include <takatori/statement/empty.h>

#include <mizugaki/ast/statement/empty_statement.h>

#include "test_parent.h"

namespace mizugaki::analyzer::details {

using namespace ::mizugaki::analyzer::testing;

class analyze_statement_dml_test : public test_parent {};

TEST_F(analyze_statement_dml_test, empty_statement) {
    auto r = analyze_statement(context(), ast::statement::empty_statement {});
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative);
    auto&& stmt = *alternative;
    ASSERT_EQ(stmt->kind(), tstatement::statement_kind::empty);
}

} // namespace mizugaki::analyzer::details
