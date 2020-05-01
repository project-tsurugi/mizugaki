#include <mizugaki/translator/details/type_info_translator.h>

#include <gtest/gtest.h>

#include <takatori/type/primitive.h>
#include <takatori/type/character.h>

#include <shakujo/common/core/type/Int.h>
#include <shakujo/common/core/type/Float.h>
#include <shakujo/common/core/type/Char.h>
#include <shakujo/common/core/type/String.h>
#include <shakujo/common/core/type/Bool.h>
#include <shakujo/common/core/type/Null.h>
#include <shakujo/common/core/type/Error.h>

#include <mizugaki/translator/testing/utils.h>

namespace mizugaki::translator::details {

using namespace ::mizugaki::translator::testing;
using code = shakujo_translator_diagnostic_code;

namespace tt = ::takatori::type;

namespace tinfo = ::shakujo::common::core::type;

class type_info_translator_test : public ::testing::Test {
public:
    shakujo_translator::impl entry { new_translator_impl() };
    shakujo_translator_context::impl context { new_context_impl() };
    type_info_translator engine { entry.initialize(context) };
};

TEST_F(type_info_translator_test, int8) {
    auto r = engine.process(tinfo::Int(8), {});
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, tt::int1());
}

TEST_F(type_info_translator_test, int16) {
    auto r = engine.process(tinfo::Int(16), {});
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, tt::int2());
}

TEST_F(type_info_translator_test, int32) {
    auto r = engine.process(tinfo::Int(32), {});
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, tt::int4());
}

TEST_F(type_info_translator_test, int64) {
    auto r = engine.process(tinfo::Int(64), {});
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, tt::int8());
}

TEST_F(type_info_translator_test, float32) {
    auto r = engine.process(tinfo::Float(32), {});
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, tt::float4 ());
}

TEST_F(type_info_translator_test, float64) {
    auto r = engine.process(tinfo::Float(64), {});
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, tt::float8 ());
}

TEST_F(type_info_translator_test, char) {
    auto r = engine.process(tinfo::Char(10), {});
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, tt::character(10));
}

TEST_F(type_info_translator_test, varchar) {
    auto r = engine.process(tinfo::Char(true, 1024), {});
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, tt::character(tt::varying, 1024));
}

TEST_F(type_info_translator_test, string) {
    auto r = engine.process(tinfo::String(), {});
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, tt::character(tt::varying));
}

TEST_F(type_info_translator_test, bool) {
    auto r = engine.process(tinfo::Bool(), {});
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, tt::boolean());
}

TEST_F(type_info_translator_test, null) {
    auto r = engine.process(tinfo::Null(), {});
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, tt::unknown());
}

TEST_F(type_info_translator_test, invalid) {
    auto r = engine.process(tinfo::Error(), {});
    EXPECT_FALSE(r);
    EXPECT_TRUE(occurred(code::unsupported_type, entry.diagnostics()));
}

} // namespace mizugaki::translator::details
