#include <mizugaki/translator/details/type_translator.h>

#include <gtest/gtest.h>

#include <takatori/type/primitive.h>
#include <takatori/type/decimal.h>

#include <takatori/type/character.h>
#include <takatori/type/octet.h>

#include <takatori/type/date.h>
#include <takatori/type/time_of_day.h>
#include <takatori/type/time_point.h>

#include <shakujo/model/IRFactory.h>

#include <mizugaki/translator/testing/utils.h>

namespace mizugaki::translator::details {

using namespace ::mizugaki::translator::testing;
using code = shakujo_translator_code;

namespace tt = ::takatori::type;

class type_translator_test : public ::testing::Test {
public:
    shakujo_translator::impl entry { new_translator_impl() };
    shakujo_translator_options options { new_options() };
    type_translator engine { entry.initialize(options) };
    
    ::shakujo::model::IRFactoryBase f;
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

TEST_F(type_translator_test, decimal) {
    auto r = engine.process(*f.DecimalType());
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, tt::decimal(options.default_decimal_precision(), 0));
}

TEST_F(type_translator_test, decimal_precision) {
    auto r = engine.process(*f.DecimalType(10));
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, tt::decimal(10, 0));
}

TEST_F(type_translator_test, decimal_scale) {
    auto r = engine.process(*f.DecimalType(10, 2));
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, tt::decimal(10, 2));
}

TEST_F(type_translator_test, decimal_dont_care) {
    auto t = f.DecimalType(::shakujo::model::type::DecimalType::dont_care, ::shakujo::model::type::DecimalType::dont_care);
    auto r = engine.process(*t);
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, tt::decimal({}, {}));
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

TEST_F(type_translator_test, varchar_flexible) {
    auto r = engine.process(*f.VarCharType(::shakujo::model::type::VarCharType::dont_care));
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, tt::character(tt::varying, {}));
}

TEST_F(type_translator_test, string) {
    auto r = engine.process(*f.StringType());
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, tt::character(tt::varying));
}

TEST_F(type_translator_test, binary) {
    auto r = engine.process(*f.BinaryType(10));
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, tt::octet(10));
}

TEST_F(type_translator_test, varbinary) {
    auto r = engine.process(*f.VarBinaryType(1024));
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, tt::octet(tt::varying, 1024));
}

TEST_F(type_translator_test, varbinary_flexible) {
    auto r = engine.process(*f.VarBinaryType(::shakujo::model::type::VarBinaryType::dont_care));
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, tt::octet(tt::varying, {}));
}

TEST_F(type_translator_test, date) {
    auto r = engine.process(*f.DateType());
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, tt::date());
}

TEST_F(type_translator_test, time) {
    auto r = engine.process(*f.TimeType());
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, tt::time_of_day());
}

TEST_F(type_translator_test, time_with_time_zone) {
    auto r = engine.process(*f.TimeType(true));
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, tt::time_of_day(tt::with_time_zone));
}

TEST_F(type_translator_test, timestamp) {
    auto r = engine.process(*f.TimestampType());
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, tt::time_point());
}

TEST_F(type_translator_test, timestamp_with_time_zone) {
    auto r = engine.process(*f.TimestampType(true));
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, tt::time_point(tt::with_time_zone));
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
