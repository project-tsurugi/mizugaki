#include <mizugaki/translator/details/type_translator.h>

#include <gtest/gtest.h>

#include <takatori/type/primitive.h>
#include <takatori/type/character.h>

#include <shakujo/model/IRFactory.h>

#include <mizugaki/translator/testing/utils.h>

namespace mizugaki::translator::details {

using namespace ::mizugaki::translator::testing;
using code = shakujo_translator_diagnostic_code;

namespace tt = ::takatori::type;

class type_translator_test : public ::testing::Test {
public:
    shakujo_translator::impl entry { new_translator_impl() };
    shakujo_translator_context::impl context { new_context_impl() };
    type_translator engine { entry.initialize(context) };
    
    ::shakujo::model::IRFactory f;
};

TEST_F(type_translator_test, int32) {
    auto r = engine.process(*f.Int32Type());
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, tt::int4());
}

TEST_F(type_translator_test, int64) {
    auto r = engine.process(*f.Int64Type());
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, tt::int8());
}

TEST_F(type_translator_test, float32) {
    auto r = engine.process(*f.Float32Type());
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, tt::float4 ());
}

TEST_F(type_translator_test, float64) {
    auto r = engine.process(*f.Float64Type());
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, tt::float8 ());
}

TEST_F(type_translator_test, char) {
    auto r = engine.process(*f.CharType(10));
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, tt::character(10));
}

TEST_F(type_translator_test, varchar) {
    auto r = engine.process(*f.VarCharType(1024));
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, tt::character(tt::varying, 1024));
}

TEST_F(type_translator_test, string) {
    auto r = engine.process(*f.StringType());
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, tt::character(tt::varying));
}

TEST_F(type_translator_test, bool) {
    auto r = engine.process(*f.BooleanType());
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, tt::boolean());
}

TEST_F(type_translator_test, null) {
    auto r = engine.process(*f.NullType());
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, tt::unknown());
}

TEST_F(type_translator_test, invalid) {
    auto r = engine.process(*f.RelationType());
    EXPECT_FALSE(r);
    EXPECT_TRUE(occurred(code::unsupported_type, entry.diagnostics()));
}

} // namespace mizugaki::translator::details
