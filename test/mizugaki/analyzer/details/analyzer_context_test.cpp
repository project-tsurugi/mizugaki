#include <mizugaki/analyzer/details/analyzer_context.h>

#include <gtest/gtest.h>

#include <takatori/type/primitive.h>

#include <yugawara/binding/factory.h>

#include "test_parent.h"

namespace mizugaki::analyzer::details {

using namespace ::mizugaki::analyzer::testing;

class analyzer_context_test : public test_parent {};

TEST_F(analyzer_context_test, type_of) {
    auto&& ctxt = context();
    auto v = vd("v", ttype::int8 {});

    auto type = ctxt.type_of(v, {});
    ASSERT_TRUE(type);
    EXPECT_EQ(*type, ttype::int8 {});
    expect_no_error();
}

TEST_F(analyzer_context_test, type_of_unresolved) {
    auto&& ctxt = context();
    auto v = ::yugawara::binding::factory {}.stream_variable("v");

    auto type = ctxt.type_of(v, {});
    EXPECT_FALSE(type);
    EXPECT_EQ(count_error(), 1);
}

} // namespace mizugaki::analyzer::details
