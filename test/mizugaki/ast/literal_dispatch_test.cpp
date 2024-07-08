#include <mizugaki/ast/literal/dispatch.h>

#include <gtest/gtest.h>

#include "utils.h"

namespace mizugaki::ast::literal {

using namespace ::mizugaki::ast::testing;

class literal_dispatch_test : public ::testing::Test {};

TEST_F(literal_dispatch_test, simple) {
    struct cb {
        int operator()(boolean const&) { return 0; }
        int operator()(numeric const&) { return 1; }
        int operator()(literal const&) { return -1; }
    };
    cb c;
    EXPECT_EQ(dispatch(c, as_lvref<literal>(boolean { true })), 0);
    EXPECT_EQ(dispatch(c, as_lvref<literal>(numeric { kind::exact_numeric, "1" })), 1);
    EXPECT_EQ(dispatch(c, as_lvref<literal>(numeric { kind::approximate_numeric, "2" })), 1);
    EXPECT_EQ(dispatch(c, as_lvref<literal>(string { kind::character_string, "3" })), -1);
}

} // namespace mizugaki::ast
