#include <mizugaki/ast/statement/dispatch.h>

#include <gtest/gtest.h>

#include <mizugaki/ast/type/simple.h>
#include <mizugaki/ast/query/table_reference.h>

#include "utils.h"

namespace mizugaki::ast::statement {

using namespace ::mizugaki::ast::testing;

class statement_dispatch_test : public ::testing::Test {};

TEST_F(statement_dispatch_test, statement) {
    struct cb {
        int operator()(empty_statement const&) { return 0; }
        int operator()(select_statement const&) { return 1; }
        int operator()(statement const&) { return -1; }
    };
    cb c;
    EXPECT_EQ(dispatch(c, as_lvref(empty_statement {})), 0);
    EXPECT_EQ(dispatch(c, as_lvref(select_statement { query::table_reference { id() } })), 1);
    EXPECT_EQ(dispatch(c, as_lvref(delete_statement { id() })), -1);
}

TEST_F(statement_dispatch_test, table_element) {
    struct cb {
        int operator()(column_definition const&) { return 0; }
        int operator()(table_constraint_definition const&) { return 1; }
        int operator()(table_element const&) { return -1; }
    };
    cb c;
    EXPECT_EQ(dispatch(c, as_lvref(column_definition { id(), type::simple { type::kind::integer } })), 0);
    EXPECT_EQ(dispatch(c, as_lvref(table_constraint_definition { simple_constraint { constraint_kind::not_null } })), 1);
}

TEST_F(statement_dispatch_test, constraint) {
    struct cb {
        int operator()(simple_constraint const&) { return 0; }
        int operator()(expression_constraint const&) { return 1; }
        int operator()(constraint const&) { return -1; }
    };
    cb c;
    EXPECT_EQ(dispatch(c, as_lvref(simple_constraint { constraint_kind::not_null })), 0);
    EXPECT_EQ(dispatch(c, as_lvref(expression_constraint { constraint_kind::check, vref() })), 1);
    EXPECT_EQ(dispatch(c, as_lvref(key_constraint { constraint_kind::primary_key, {} })), -1);
}

} // namespace mizugaki::ast
