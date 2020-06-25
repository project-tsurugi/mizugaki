#include <mizugaki/ast/type/dispatch.h>

#include <gtest/gtest.h>

#include "utils.h"

namespace mizugaki::ast::type {

using namespace ::mizugaki::ast::testing;

class type_dispatch_test : public ::testing::Test {};

TEST_F(type_dispatch_test, type) {
    struct cb {
        int operator()(simple const&) { return 0; }
        int operator()(decimal const&) { return 1; }
        int operator()(type const&) { return -1; }
    };
    cb c;
    EXPECT_EQ(dispatch(c, as_lvref(simple { kind::boolean })), 0);
    EXPECT_EQ(dispatch(c, as_lvref(decimal { kind::decimal, 18, 2 })), 1);
    EXPECT_EQ(dispatch(c, as_lvref(user_defined { id() })), -1);
}

} // namespace mizugaki::ast
