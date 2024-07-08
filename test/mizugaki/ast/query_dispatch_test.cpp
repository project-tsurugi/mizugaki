#include <mizugaki/ast/query/dispatch.h>

#include <gtest/gtest.h>

#include "utils.h"

namespace mizugaki::ast::query {

using namespace ::mizugaki::ast::testing;

class query_dispatch_test : public ::testing::Test {};

TEST_F(query_dispatch_test, expression) {
    struct cb {
        int operator()(table_reference const&) { return 0; }
        int operator()(table_value_constructor const&) { return 1; }
        int operator()(expression const&) { return -1; }
    };
    cb c;
    EXPECT_EQ(dispatch(c, as_lvref<expression>(table_reference { id() })), 0);
    EXPECT_EQ(dispatch(c, as_lvref<expression>(table_value_constructor {})), 1);
}

TEST_F(query_dispatch_test, grouping_element) {
    struct cb {
        int operator()(grouping_column const&) { return 0; }
        int operator()(grouping_element const&) { return -1; }
    };
    cb c;
    EXPECT_EQ(dispatch(c, as_lvref<grouping_element>(grouping_column { id() })), 0);
}

TEST_F(query_dispatch_test, select_element) {
    struct cb {
        int operator()(select_column const&) { return 0; }
        int operator()(select_asterisk const&) { return 1; }
        int operator()(select_element const&) { return -1; }
    };
    cb c;
    EXPECT_EQ(dispatch(c, as_lvref<select_element>(select_column { vref() })), 0);
    EXPECT_EQ(dispatch(c, as_lvref<select_element>(select_asterisk { vref() })), 1);
}

} // namespace mizugaki::ast
