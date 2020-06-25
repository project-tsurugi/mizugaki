#include <mizugaki/ast/table/dispatch.h>

#include <gtest/gtest.h>

#include <mizugaki/ast/table/subquery.h>
#include <mizugaki/ast/query/table_reference.h>

#include "utils.h"

namespace mizugaki::ast::table {

using namespace ::mizugaki::ast::testing;

class table_dispatch_test : public ::testing::Test {};

TEST_F(table_dispatch_test, expression) {
    struct cb {
        int operator()(table_reference const&) { return 0; }
        int operator()(subquery const&) { return 1; }
        int operator()(expression const&) { return -1; }
    };
    cb c;
    EXPECT_EQ(dispatch(c, as_lvref(table_reference { id() })), 0);
    EXPECT_EQ(dispatch(c, as_lvref(subquery { query::table_reference { id() }, correlation_clause { id() }  })), 1);
    EXPECT_EQ(dispatch(c, as_lvref(unnest { vref(), correlation_clause { id() } })), -1);
}

TEST_F(table_dispatch_test, join_specification) {
    struct cb {
        int operator()(join_condition const&) { return 0; }
        int operator()(join_specification const&) { return -1; }
    };
    cb c;
    EXPECT_EQ(dispatch(c, as_lvref(join_condition { vref() })), 0);
    EXPECT_EQ(dispatch(c, as_lvref(join_columns {})), -1);
}

} // namespace mizugaki::ast
