#include <mizugaki/ast/scalar/dispatch.h>

#include <gtest/gtest.h>

#include "utils.h"

namespace mizugaki::ast::scalar {

using namespace ::mizugaki::ast::testing;

class scalar_dispatch_test : public ::testing::Test {};

TEST_F(scalar_dispatch_test, expression) {
    struct cb {
        int operator()(variable_reference const&) { return 0; }
        int operator()(unary_expression const&) { return 1; }
        int operator()(expression const&) { return -1; }
    };
    cb c;
    EXPECT_EQ(dispatch(c, as_lvref(variable_reference { id() })), 0);
    EXPECT_EQ(dispatch(c, as_lvref(unary_expression { unary_operator::plus, vref() })), 1);
    EXPECT_EQ(dispatch(c, as_lvref(comparison_predicate { vref(), comparison_operator::equals, vref() })), -1);
}

} // scalarspace mizugaki::ast
