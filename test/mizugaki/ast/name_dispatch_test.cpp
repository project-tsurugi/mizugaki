#include <mizugaki/ast/name/dispatch.h>

#include <gtest/gtest.h>

#include "utils.h"

namespace mizugaki::ast::name {

using namespace ::mizugaki::ast::testing;

class name_dispatch_test : public ::testing::Test {};

TEST_F(name_dispatch_test, simple) {
    struct cb {
        int operator()(simple const&) { return 0; }
        int operator()(qualified const&) { return 1; }
        int operator()(name const&) { return -1; }
    };
    cb c;
    EXPECT_EQ(dispatch(c, as_lvref<name>(simple { "a" })), 0);
    EXPECT_EQ(dispatch(c, as_lvref<name>(qualified { simple { "a" }, simple { "b" } })), 1);
}

} // namespace mizugaki::ast
