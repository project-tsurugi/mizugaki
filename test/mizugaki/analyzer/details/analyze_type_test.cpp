#include <mizugaki/analyzer/details/analyze_type.h>

#include <gtest/gtest.h>

#include <takatori/type/primitive.h>
#include <takatori/type/decimal.h>
#include <takatori/type/character.h>
#include <takatori/type/octet.h>
#include <takatori/type/bit.h>
#include <takatori/type/date.h>
#include <takatori/type/time_of_day.h>
#include <takatori/type/time_point.h>
#include <takatori/type/datetime_interval.h>

#include <mizugaki/ast/type/simple.h>
#include <mizugaki/ast/type/character_string.h>
#include <mizugaki/ast/type/bit_string.h>
#include <mizugaki/ast/type/octet_string.h>
#include <mizugaki/ast/type/decimal.h>
#include <mizugaki/ast/type/binary_numeric.h>
#include <mizugaki/ast/type/datetime.h>
#include <mizugaki/ast/type/interval.h>
#include <mizugaki/ast/type/user_defined.h>

#include "test_parent.h"

namespace mizugaki::analyzer::details {

using namespace ::mizugaki::analyzer::testing;
using error_code = analyzer_context::diagnostic_type::code_type;

class analyze_type_test : public test_parent {};

static constexpr std::size_t flexible = ast::type::character_string::flexible_length;

TEST_F(analyze_type_test, unknown) {
    auto r = analyze_type(
            context(),
            ast::type::simple {
                    ast::type::kind::unknown,
            });
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (ttype::unknown {}));
}

TEST_F(analyze_type_test, character) {
    auto r = analyze_type(
            context(),
            ast::type::character_string {
                    ast::type::kind::character,
            });
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (ttype::character { 1 }));
}

TEST_F(analyze_type_test, character_length) {
    auto r = analyze_type(
            context(),
            ast::type::character_string {
                    ast::type::kind::character,
                    16,
            });
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (ttype::character { 16 }));
}

TEST_F(analyze_type_test, character_flexible) {
    auto r = analyze_type(
            context(),
            ast::type::character_string {
                    ast::type::kind::character,
                    flexible,
            });
    EXPECT_FALSE(r) << diagnostics();
    EXPECT_TRUE(find_error(error_code::flexible_length_is_not_supported));
}

TEST_F(analyze_type_test, character_length_zero) {
    auto r = analyze_type(
            context(),
            ast::type::character_string {
                    ast::type::kind::character,
                    0,
            });
    EXPECT_FALSE(r) << diagnostics();
    EXPECT_TRUE(find_error(sql_analyzer_code::invalid_type_length));
}

TEST_F(analyze_type_test, character_varying) {
    auto r = analyze_type(
            context(),
            ast::type::character_string {
                    ast::type::kind::character_varying,
            });
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (ttype::character { ttype::varying, {} }));
}

TEST_F(analyze_type_test, character_varying_length) {
    auto r = analyze_type(
            context(),
            ast::type::character_string {
                    ast::type::kind::character_varying,
                    10,
            });
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (ttype::character { ttype::varying, 10 }));
}

TEST_F(analyze_type_test, character_varying_flexible) {
    auto r = analyze_type(
            context(),
            ast::type::character_string {
                    ast::type::kind::character_varying,
                    flexible,
            });
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (ttype::character { ttype::varying, std::nullopt }));
}

TEST_F(analyze_type_test, character_varying_length_zero) {
    auto r = analyze_type(
            context(),
            ast::type::character_string {
                    ast::type::kind::character_varying,
                    0,
            });
    EXPECT_FALSE(r) << diagnostics();
    EXPECT_TRUE(find_error(sql_analyzer_code::invalid_type_length));
}

TEST_F(analyze_type_test, bit) {
    auto r = analyze_type(
            context(),
            ast::type::bit_string {
                    ast::type::kind::bit,
            });
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (ttype::bit { 1 }));
}

TEST_F(analyze_type_test, bit_length) {
    auto r = analyze_type(
            context(),
            ast::type::bit_string {
                    ast::type::kind::bit,
                    16,
            });
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (ttype::bit { 16 }));
}

TEST_F(analyze_type_test, bit_flexible) {
    auto r = analyze_type(
            context(),
            ast::type::bit_string {
                    ast::type::kind::bit,
                    flexible,
            });
    EXPECT_FALSE(r);
    EXPECT_TRUE(find_error(error_code::flexible_length_is_not_supported));
}

TEST_F(analyze_type_test, bit_length_zero) {
    auto r = analyze_type(
            context(),
            ast::type::bit_string {
                    ast::type::kind::bit,
                    0,
            });
    EXPECT_FALSE(r);
    EXPECT_TRUE(find_error(sql_analyzer_code::invalid_type_length));
}

TEST_F(analyze_type_test, bit_varying) {
    auto r = analyze_type(
            context(),
            ast::type::bit_string {
                    ast::type::kind::bit_varying,
            });
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (ttype::bit { ttype::varying, {} }));
}

TEST_F(analyze_type_test, bit_varying_length) {
    auto r = analyze_type(
            context(),
            ast::type::bit_string {
                    ast::type::kind::bit_varying,
                    10,
            });
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (ttype::bit { ttype::varying, 10 }));
}

TEST_F(analyze_type_test, bit_varying_flexible) {
    auto r = analyze_type(
            context(),
            ast::type::bit_string {
                    ast::type::kind::bit_varying,
                    flexible,
            });
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (ttype::bit { ttype::varying, std::nullopt }));
}

TEST_F(analyze_type_test, bit_varying_length_zero) {
    auto r = analyze_type(
            context(),
            ast::type::bit_string {
                    ast::type::kind::bit_varying,
                    0,
            });
    EXPECT_FALSE(r);
    EXPECT_TRUE(find_error(sql_analyzer_code::invalid_type_length));
}

TEST_F(analyze_type_test, octet) {
    auto r = analyze_type(
            context(),
            ast::type::octet_string {
                    ast::type::kind::octet,
            });
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (ttype::octet { 1 }));
}

TEST_F(analyze_type_test, octet_length) {
    auto r = analyze_type(
            context(),
            ast::type::octet_string {
                    ast::type::kind::octet,
                    16,
            });
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (ttype::octet { 16 }));
}

TEST_F(analyze_type_test, octet_flexible) {
    auto r = analyze_type(
            context(),
            ast::type::octet_string {
                    ast::type::kind::octet,
                    flexible,
            });
    EXPECT_FALSE(r);
    EXPECT_TRUE(find_error(error_code::flexible_length_is_not_supported));
}

TEST_F(analyze_type_test, octet_length_zero) {
    auto r = analyze_type(
            context(),
            ast::type::octet_string {
                    ast::type::kind::octet,
                    0,
            });
    EXPECT_FALSE(r);
    EXPECT_TRUE(find_error(sql_analyzer_code::invalid_type_length));
}

TEST_F(analyze_type_test, octet_varying) {
    auto r = analyze_type(
            context(),
            ast::type::octet_string {
                    ast::type::kind::octet_varying,
            });
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (ttype::octet { ttype::varying, {} }));
}

TEST_F(analyze_type_test, octet_varying_length) {
    auto r = analyze_type(
            context(),
            ast::type::octet_string {
                    ast::type::kind::octet_varying,
                    10,
            });
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (ttype::octet { ttype::varying, 10 }));
}

TEST_F(analyze_type_test, octet_varying_flexible) {
    auto r = analyze_type(
            context(),
            ast::type::octet_string {
                    ast::type::kind::octet_varying,
                    flexible,
            });
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (ttype::octet { ttype::varying, std::nullopt }));
}

TEST_F(analyze_type_test, octet_varying_length_zero) {
    auto r = analyze_type(
            context(),
            ast::type::octet_string {
                    ast::type::kind::octet_varying,
                    0,
            });
    EXPECT_FALSE(r);
    EXPECT_TRUE(find_error(sql_analyzer_code::invalid_type_length));
}

TEST_F(analyze_type_test, numeric) {
    options_.default_decimal_precision() = 20;
    auto r = analyze_type(
            context(),
            ast::type::numeric {
                    ast::type::kind::numeric,
            });
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (ttype::decimal { 20, 0 }));
}

TEST_F(analyze_type_test, decimal) {
    options_.default_decimal_precision() = 20;
    auto r = analyze_type(
            context(),
            ast::type::decimal {
                    ast::type::kind::decimal,
            });
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (ttype::decimal { 20, 0 }));
}

TEST_F(analyze_type_test, decimal_precision) {
    auto r = analyze_type(
            context(),
            ast::type::decimal {
                    ast::type::kind::decimal,
                    16,
            });
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (ttype::decimal { 16, 0 }));
}

TEST_F(analyze_type_test, decimal_scale) {
    auto r = analyze_type(
            context(),
            ast::type::decimal {
                    ast::type::kind::decimal,
                    16,
                    2,
            });
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (ttype::decimal { 16, 2 }));
}

TEST_F(analyze_type_test, decimal_flexible_precision) {
    auto r = analyze_type(
            context(),
            ast::type::decimal {
                    ast::type::kind::decimal,
                    flexible,
            });
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (ttype::decimal { {}, 0 }));
}

TEST_F(analyze_type_test, decimal_flexible_both) {
    auto r = analyze_type(
            context(),
            ast::type::decimal {
                    ast::type::kind::decimal,
                    flexible,
                    flexible,
            });
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (ttype::decimal { {}, {} }));
}

TEST_F(analyze_type_test, decimal_flexible_with_scale) {
    auto r = analyze_type(
            context(),
            // DECIMAL(*, 2)
            ast::type::decimal {
                    ast::type::kind::decimal,
                    flexible,
                    2,
            });
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (ttype::decimal { {}, 2, }));
}

TEST_F(analyze_type_test, decimal_precision_zero) {
    auto r = analyze_type(
            context(),
            ast::type::decimal {
                    ast::type::kind::decimal,
                    0,
            });
    EXPECT_FALSE(r);
    EXPECT_TRUE(find_error(sql_analyzer_code::invalid_type_length));
}

TEST_F(analyze_type_test, decimal_precision_too_large) {
    options_.max_decimal_precision() = 20;
    auto r = analyze_type(
            context(),
            ast::type::decimal {
                    ast::type::kind::decimal,
                    21,
            });
    EXPECT_FALSE(r);
    EXPECT_TRUE(find_error(sql_analyzer_code::invalid_type_length));
}

TEST_F(analyze_type_test, decimal_scale_too_large) {
    auto r = analyze_type(
            context(),
            ast::type::decimal {
                    ast::type::kind::decimal,
                    10,
                    11,
            });
    EXPECT_FALSE(r);
    EXPECT_TRUE(find_error(sql_analyzer_code::invalid_numeric_scale));
}

TEST_F(analyze_type_test, tiny_integer) {
    auto r = analyze_type(
            context(),
            ast::type::simple {
                    ast::type::kind::tiny_integer,
            });
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (ttype::int1 {}));
}

TEST_F(analyze_type_test, small_integer) {
    auto r = analyze_type(
            context(),
            ast::type::simple {
                    ast::type::kind::small_integer,
            });
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (ttype::int2 {}));
}

TEST_F(analyze_type_test, integer) {
    auto r = analyze_type(
            context(),
            ast::type::simple {
                    ast::type::kind::integer,
            });
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (ttype::int4 {}));
}

TEST_F(analyze_type_test, big_integer) {
    auto r = analyze_type(
            context(),
            ast::type::simple {
                    ast::type::kind::big_integer,
            });
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (ttype::int8 {}));
}

TEST_F(analyze_type_test, float) {
    auto r = analyze_type(
            context(),
            ast::type::simple {
                    ast::type::kind::float_,
            });
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (ttype::float4 {}));
}

TEST_F(analyze_type_test, real) {
    auto r = analyze_type(
            context(),
            ast::type::simple {
                    ast::type::kind::float_,
            });
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (ttype::float4 {}));
}

TEST_F(analyze_type_test, double_precision) {
    auto r = analyze_type(
            context(),
            ast::type::simple {
                    ast::type::kind::double_precision,
            });
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (ttype::float8 {}));
}

TEST_F(analyze_type_test, integer_tiny) {
    auto r = analyze_type(
            context(),
            ast::type::binary_numeric {
                    ast::type::kind::binary_integer,
                    7,
            });
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (ttype::int1 {}));
}

TEST_F(analyze_type_test, integer_small) {
    auto r = analyze_type(
            context(),
            ast::type::binary_numeric {
                    ast::type::kind::binary_integer,
                    15,
            });
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (ttype::int2 {}));
}

TEST_F(analyze_type_test, integer_medium) {
    auto r = analyze_type(
            context(),
            ast::type::binary_numeric {
                    ast::type::kind::binary_integer,
                    31,
            });
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (ttype::int4 {}));
}

TEST_F(analyze_type_test, integer_big) {
    auto r = analyze_type(
            context(),
            ast::type::binary_numeric {
                    ast::type::kind::binary_integer,
                    63,
            });
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (ttype::int8 {}));
}

TEST_F(analyze_type_test, integer_flexible) {
    auto r = analyze_type(
            context(),
            ast::type::binary_numeric {
                    ast::type::kind::binary_integer,
                    flexible,
            });
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (ttype::int8 {}));
}

TEST_F(analyze_type_test, binary_numeric_length_zero) {
    auto r = analyze_type(
            context(),
            ast::type::binary_numeric {
                    ast::type::kind::binary_integer,
                    0,
            });
    EXPECT_FALSE(r);
    EXPECT_TRUE(find_error(sql_analyzer_code::invalid_type_length));
}

TEST_F(analyze_type_test, binary_numeric_length_too_large) {
    auto r = analyze_type(
            context(),
            ast::type::binary_numeric {
                    ast::type::kind::binary_integer,
                    options_.max_binary_integer8_precision() + 1,
            });
    EXPECT_FALSE(r);
    EXPECT_TRUE(find_error(sql_analyzer_code::invalid_type_length));
}

TEST_F(analyze_type_test, float_single) {
    auto r = analyze_type(
            context(),
            ast::type::binary_numeric {
                    ast::type::kind::binary_float,
                    sql_analyzer_options::default_max_binary_float4_precision,
            });
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (ttype::float4 {}));
}

TEST_F(analyze_type_test, float_double) {
    auto r = analyze_type(
            context(),
            ast::type::binary_numeric {
                    ast::type::kind::binary_float,
                    sql_analyzer_options::default_max_binary_float8_precision,
            });
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (ttype::float8 {}));
}

TEST_F(analyze_type_test, float_flexible) {
    auto r = analyze_type(
            context(),
            ast::type::binary_numeric {
                    ast::type::kind::binary_float,
                    flexible,
            });
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (ttype::float8 {}));
}

TEST_F(analyze_type_test, binary_float_length_zero) {
    auto r = analyze_type(
            context(),
            ast::type::binary_numeric {
                    ast::type::kind::binary_float,
                    0,
            });
    EXPECT_FALSE(r);
    EXPECT_TRUE(find_error(sql_analyzer_code::invalid_type_length));
}

TEST_F(analyze_type_test, binary_float_length_too_large) {
    auto r = analyze_type(
            context(),
            ast::type::binary_numeric {
                    ast::type::kind::binary_float,
                    options_.max_binary_float8_precision() + 1,
            });
    EXPECT_FALSE(r);
    EXPECT_TRUE(find_error(sql_analyzer_code::invalid_type_length));
}

TEST_F(analyze_type_test, boolean) {
    auto r = analyze_type(
            context(),
            ast::type::simple {
                    ast::type::kind::boolean,
            });
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (ttype::boolean {}));
}

TEST_F(analyze_type_test, date) {
    auto r = analyze_type(
            context(),
            ast::type::simple {
                    ast::type::kind::date,
            });
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (ttype::date {}));
}

TEST_F(analyze_type_test, time) {
    auto r = analyze_type(
            context(),
            ast::type::datetime {
                    ast::type::kind::time,
            });
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (ttype::time_of_day {}));
}

TEST_F(analyze_type_test, time_timezone) {
    auto r = analyze_type(
            context(),
            ast::type::datetime {
                    ast::type::kind::time,
                    true,
            });
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (ttype::time_of_day { ttype::with_time_zone }));
}

TEST_F(analyze_type_test, timestamp) {
    auto r = analyze_type(
            context(),
            ast::type::datetime {
                    ast::type::kind::timestamp,
            });
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (ttype::time_point {}));
}

TEST_F(analyze_type_test, timestamp_timezone) {
    auto r = analyze_type(
            context(),
            ast::type::datetime {
                    ast::type::kind::timestamp,
                    true,
            });
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_EQ(*r, (ttype::time_point { ttype::with_time_zone }));
}

TEST_F(analyze_type_test, user_defined_unsupported) {
    auto r = analyze_type(
            context(),
            ast::type::user_defined {
                    id("undefined"),
            });
    EXPECT_FALSE(r);
    EXPECT_TRUE(find_error(sql_analyzer_code::unsupported_feature));
}

} // namespace mizugaki::analyzer::details
