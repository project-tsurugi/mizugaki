#include <mizugaki/translator/details/value_info_translator.h>

#include <numeric>

#include <gtest/gtest.h>

#include <takatori/value/primitive.h>
#include <takatori/value/character.h>

#include <shakujo/common/core/value/Int.h>
#include <shakujo/common/core/value/Float.h>
#include <shakujo/common/core/value/String.h>
#include <shakujo/common/core/value/Bool.h>
#include <shakujo/common/core/value/Null.h>
#include <shakujo/common/core/value/Error.h>

#include <mizugaki/translator/testing/utils.h>

namespace mizugaki::translator::details {

using namespace ::mizugaki::translator::testing;
using code = shakujo_translator_code;

namespace tv = ::takatori::value;

namespace vinfo = ::shakujo::common::core::value;

class value_info_translator_test : public ::testing::Test {
public:
    shakujo_translator::impl entry { new_translator_impl() };
    shakujo_translator_options options { new_options() };
    value_info_translator engine { entry.initialize(options) };
};

TEST_F(value_info_translator_test, int32) {
    auto r = engine.process(vinfo::Int(-1), {});
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, tv::int4(-1));
}

TEST_F(value_info_translator_test, int64) {
    std::int64_t v = std::numeric_limits<std::int32_t>::max();
    auto r = engine.process(vinfo::Int(v + 1), {});
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, tv::int8(v + 1));
}

TEST_F(value_info_translator_test, float64) {
    auto r = engine.process(vinfo::Float(1.5), {});
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, tv::float8(1.5));
}

TEST_F(value_info_translator_test, string) {
    auto r = engine.process(vinfo::String("Hello, world!"), {});
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, tv::character("Hello, world!"));
}

TEST_F(value_info_translator_test, bool) {
    auto r = engine.process(vinfo::Bool(true), {});
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, tv::boolean(true));
}

TEST_F(value_info_translator_test, null) {
    auto r = engine.process(vinfo::Null(), {});
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, tv::unknown(tv::unknown_kind::null));
}

TEST_F(value_info_translator_test, invalid) {
    auto r = engine.process(vinfo::Error(), {});
    EXPECT_FALSE(r);
    EXPECT_TRUE(occurred(code::unsupported_value, entry.diagnostics()));
}

} // namespace mizugaki::translator::details
