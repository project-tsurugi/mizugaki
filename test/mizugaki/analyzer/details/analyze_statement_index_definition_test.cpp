#include <mizugaki/analyzer/details/analyze_statement.h>

#include <gtest/gtest.h>

#include <takatori/value/primitive.h>
#include <takatori/type/primitive.h>

#include <takatori/statement/empty.h>
#include <takatori/statement/create_index.h>

#include <yugawara/binding/extract.h>

#include <yugawara/storage/table.h>
#include <yugawara/storage/index.h>
#include <yugawara/storage/column.h>

#include <mizugaki/ast/statement/index_definition.h>
#include <mizugaki/ast/statement/key_constraint.h>

#include "test_parent.h"

namespace mizugaki::analyzer::details {

using namespace ::mizugaki::analyzer::testing;

using ::yugawara::binding::extract;

class analyze_statement_index_definition_test : public test_parent {
protected:
    void invalid(ast::statement::statement const& stmt) {
        trelation::graph_type graph {};
        auto r = analyze_statement(context(), stmt);
        EXPECT_TRUE(std::holds_alternative<erroneous_result_type>(r)) << diagnostics();
        EXPECT_GT(count_error(), 0);
    }

    void invalid(sql_analyzer_code code, ast::statement::statement const& stmt) {
        invalid(stmt);
        EXPECT_TRUE(find_error(code)) << diagnostics();
    }
};

TEST_F(analyze_statement_index_definition_test, simple) {
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

TEST_F(analyze_statement_index_definition_test, multiple) {
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

TEST_F(analyze_statement_index_definition_test, direction) {
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

} // namespace mizugaki::analyzer::details
