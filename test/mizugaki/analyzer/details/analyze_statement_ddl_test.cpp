#include <mizugaki/analyzer/details/analyze_statement.h>

#include <gtest/gtest.h>

#include <takatori/value/primitive.h>
#include <takatori/type/primitive.h>

#include <takatori/statement/empty.h>
#include <takatori/statement/create_index.h>
#include <takatori/statement/drop_table.h>
#include <takatori/statement/drop_index.h>

#include <yugawara/binding/extract.h>

#include <yugawara/storage/table.h>
#include <yugawara/storage/index.h>
#include <yugawara/storage/column.h>

#include <mizugaki/ast/statement/index_definition.h>
#include <mizugaki/ast/statement/drop_statement.h>
#include <mizugaki/ast/statement/key_constraint.h>

#include "test_parent.h"

namespace mizugaki::analyzer::details {

using namespace ::mizugaki::analyzer::testing;

using ::yugawara::binding::extract;

class analyze_statement_ddl_test : public test_parent {};

TEST_F(analyze_statement_ddl_test, index_definition_simple) {
    auto table = install_table("testing_table");
    auto r = analyze_statement(context(), ast::statement::index_definition {
            id("testing"),
            id("testing_table"),
            {
                    {
                            id("v"),
                    }
            },
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative);
    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::create_index);

    auto&& stmt = downcast<tstatement::create_index>(**alternative);
    EXPECT_EQ(&extract(stmt.schema()), default_schema_.get());

    auto&& index = extract<::yugawara::storage::index>(stmt.definition());
    EXPECT_EQ(index.simple_name(), "testing");

    auto&& keys = index.keys();
    ASSERT_EQ(keys.size(), 1);
    {
        auto&& key = keys[0];
        EXPECT_EQ(&key.column(), &table->columns()[1]);
        EXPECT_EQ(key.direction(), ::yugawara::storage::index::key::direction_type::ascendant);
    }

    ASSERT_EQ(index.values().size(), 0);
    ASSERT_EQ(index.features(), ::yugawara::storage::index::feature_set_type {});
}

TEST_F(analyze_statement_ddl_test, index_definition_multiple) {
    auto table = install_table("testing_table");
    auto r = analyze_statement(context(), ast::statement::index_definition {
            id("testing"),
            id("testing_table"),
            {
                    {
                            id("v"),
                    },
                    {
                        id("w"),
                    },
                    {
                        id("x"),
                    }
            },
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative);
    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::create_index);

    auto&& stmt = downcast<tstatement::create_index>(**alternative);

    auto&& index = extract<::yugawara::storage::index>(stmt.definition());
    EXPECT_EQ(index.simple_name(), "testing");

    auto&& keys = index.keys();
    ASSERT_EQ(keys.size(), 3);
    {
        auto&& key = keys[0];
        EXPECT_EQ(&key.column(), &table->columns()[1]);
        EXPECT_EQ(key.direction(), ::yugawara::storage::index::key::direction_type::ascendant);
    }
    {
        auto&& key = keys[1];
        EXPECT_EQ(&key.column(), &table->columns()[2]);
        EXPECT_EQ(key.direction(), ::yugawara::storage::index::key::direction_type::ascendant);
    }
    {
        auto&& key = keys[2];
        EXPECT_EQ(&key.column(), &table->columns()[3]);
        EXPECT_EQ(key.direction(), ::yugawara::storage::index::key::direction_type::ascendant);
    }
}

TEST_F(analyze_statement_ddl_test, index_definition_direction) {
    auto table = install_table("testing_table");
    auto r = analyze_statement(context(), ast::statement::index_definition {
            id("testing"),
            id("testing_table"),
            {
                    {
                            id("v"),
                            ast::common::ordering_specification::asc,
                    },
                    {
                            id("w"),
                            ast::common::ordering_specification::desc,
                    },
            },
    });
    auto alternative = std::get_if<statement_result_type>(&r);
    ASSERT_TRUE(alternative);
    ASSERT_EQ((*alternative)->kind(), tstatement::statement_kind::create_index);

    auto&& stmt = downcast<tstatement::create_index>(**alternative);
    EXPECT_EQ(&extract(stmt.schema()), default_schema_.get());

    auto&& index = extract<::yugawara::storage::index>(stmt.definition());
    EXPECT_EQ(index.simple_name(), "testing");

    auto&& keys = index.keys();
    ASSERT_EQ(keys.size(), 2);
    {
        auto&& key = keys[0];
        EXPECT_EQ(&key.column(), &table->columns()[1]);
        EXPECT_EQ(key.direction(), ::yugawara::storage::index::key::direction_type::ascendant);
    }
    {
        auto&& key = keys[1];
        EXPECT_EQ(&key.column(), &table->columns()[2]);
        EXPECT_EQ(key.direction(), ::yugawara::storage::index::key::direction_type::descendant);
    }
}

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
