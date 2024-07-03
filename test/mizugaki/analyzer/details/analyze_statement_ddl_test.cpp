#include <mizugaki/analyzer/details/analyze_statement.h>

#include <gtest/gtest.h>

#include <takatori/value/primitive.h>
#include <takatori/type/primitive.h>

#include <takatori/statement/empty.h>
#include <takatori/statement/drop_table.h>
#include <takatori/statement/drop_index.h>

#include <yugawara/binding/extract.h>

#include <yugawara/storage/table.h>
#include <yugawara/storage/index.h>

#include <mizugaki/ast/statement/drop_statement.h>

#include "test_parent.h"

namespace mizugaki::analyzer::details {

using namespace ::mizugaki::analyzer::testing;

using ::yugawara::binding::extract;

class analyze_statement_ddl_test : public test_parent {};

TEST_F(analyze_statement_ddl_test, drop_table_simple) {
    auto table = install_table("testing");
    auto r = analyze_statement(context(), ast::statement::drop_statement {
            ast::statement::kind::drop_table_statement,
            id("testing"),
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative);
    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::drop_table);

    auto&& stmt = downcast<tstatement::drop_table>(**alternative);
    EXPECT_EQ(&extract(stmt.schema()), default_schema_.get());

    auto&& target = extract(stmt.target());
    EXPECT_EQ(&target, table.get());
}

TEST_F(analyze_statement_ddl_test, drop_index_simple) {
    auto table = install_table("testing");
    auto index = storages_->add_index(::yugawara::storage::index {
            table,
            "testing_index",
            {
                    table->columns()[1],
            }
    });

    auto r = analyze_statement(context(), ast::statement::drop_statement {
            ast::statement::kind::drop_index_statement,
            id("testing_index"),
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative);
    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::drop_index);

    auto&& stmt = downcast<tstatement::drop_index>(**alternative);
    EXPECT_EQ(&extract(stmt.schema()), default_schema_.get());

    auto&& target = extract<::yugawara::storage::index>(stmt.target());
    EXPECT_EQ(&target, index.get());
}

} // namespace mizugaki::analyzer::details
