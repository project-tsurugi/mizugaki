#include <mizugaki/parser/sql_parser.h>

#include <gtest/gtest.h>

#include <mizugaki/ast/statement/select_statement.h>

#include <mizugaki/ast/query/query.h>
#include <mizugaki/ast/query/table_value_constructor.h>
#include <mizugaki/ast/scalar/value_constructor.h>
#include <mizugaki/ast/scalar/cast_expression.h>

#include <mizugaki/ast/type/simple.h>
#include <mizugaki/ast/type/character_string.h>
#include <mizugaki/ast/type/bit_string.h>
#include <mizugaki/ast/type/octet_string.h>
#include <mizugaki/ast/type/decimal.h>
#include <mizugaki/ast/type/binary_numeric.h>
#include <mizugaki/ast/type/datetime.h>
#include <mizugaki/ast/type/interval.h>
#include <mizugaki/ast/type/row.h>
#include <mizugaki/ast/type/user_defined.h>
#include <mizugaki/ast/type/collection.h>

#include <mizugaki/ast/name/qualified.h>

#include "utils.h"

namespace mizugaki::parser {

using namespace testing;

class sql_parser_type_test : public ::testing::Test {};

static sql_parser_result parse(std::string_view str, sql_parser parser = sql_parser {}) {
    using ::takatori::util::string_builder;
    return parser("-", string_builder {}
            << "VALUES (CAST(NULL AS " << str << "));"
            << string_builder::to_string);
}

static type::type const& extract(sql_parser::result_type const& result) {
    auto&& stmt = result.value()->statements().at(0);
    auto&& select = downcast<statement::select_statement>(*stmt);
    auto&& tv = downcast<query::table_value_constructor>(*select.expression());
    auto&& rv = downcast<scalar::value_constructor>(*tv.elements()[0]);
    auto&& le = downcast<scalar::cast_expression>(*rv.elements()[0]);
    return *le.type();
}

TEST_F(sql_parser_type_test, character) {
    auto result = parse("CHARACTER");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::character_string {
            type::kind::character,
    }));
}

TEST_F(sql_parser_type_test, char) {
    auto result = parse("CHAR");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::character_string {
            type::kind::character,
    }));
}

TEST_F(sql_parser_type_test, char_length) {
    auto result = parse("CHAR(10)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::character_string {
            type::kind::character,
            10,
    }));
}

TEST_F(sql_parser_type_test, character_varying) {
    auto result = parse("CHARACTER VARYING(10)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::character_string {
            type::kind::character_varying,
            10,
    }));
}

TEST_F(sql_parser_type_test, char_varying) {
    auto result = parse("CHAR VARYING(10)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::character_string {
            type::kind::character_varying,
            10,
    }));
}

TEST_F(sql_parser_type_test, varchar) {
    auto result = parse("VARCHAR(10)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::character_string {
            type::kind::character_varying,
            10,
    }));
}

TEST_F(sql_parser_type_test, varchar_flexible) {
    auto result = parse("VARCHAR(*)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::character_string {
            type::kind::character_varying,
            type::character_string::flexible_length,
    }));
}

TEST_F(sql_parser_type_test, bit) {
    auto result = parse("BIT");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::bit_string {
            type::kind::bit,
    }));
}

TEST_F(sql_parser_type_test, bit_length) {
    auto result = parse("BIT(16)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::bit_string {
            type::kind::bit,
            16,
    }));
}

TEST_F(sql_parser_type_test, bit_varying) {
    auto result = parse("BIT VARYING(256)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::bit_string {
            type::kind::bit_varying,
            256,
    }));
}

TEST_F(sql_parser_type_test, bit_varying_flexible) {
    auto result = parse("BIT VARYING(*)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::bit_string {
            type::kind::bit_varying,
            type::bit_string::flexible_length,
    }));
}

TEST_F(sql_parser_type_test, octet) {
    auto result = parse("BYTE");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::octet_string {
            type::kind::octet,
    }));
}

TEST_F(sql_parser_type_test, octet_length) {
    auto result = parse("BYTE(16)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::octet_string {
            type::kind::octet,
            16,
    }));
}

TEST_F(sql_parser_type_test, octet_varying) {
    auto result = parse("BYTE VARYING(256)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::octet_string {
            type::kind::octet_varying,
            256,
    }));
}

TEST_F(sql_parser_type_test, octet_varying_flexible) {
    auto result = parse("VARBYTE(*)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::octet_string {
            type::kind::octet_varying,
            type::octet_string::flexible_length,
    }));
}

TEST_F(sql_parser_type_test, numeric) {
    auto result = parse("NUMERIC");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::numeric {
            type::kind::numeric,
    }));
}

TEST_F(sql_parser_type_test, numeric_precision) {
    auto result = parse("NUMERIC(10)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::numeric {
            type::kind::numeric,
            10,
    }));
}

TEST_F(sql_parser_type_test, numeric_precision_scale) {
    auto result = parse("NUMERIC(10, 2)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::numeric {
            type::kind::numeric,
            10,
            2,
    }));
}

TEST_F(sql_parser_type_test, numeric_flexible_precision) {
    auto result = parse("NUMERIC(*)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::numeric {
            type::kind::numeric,
            type::decimal::flexible_precision
    }));
}

TEST_F(sql_parser_type_test, numeric_flexible_precision_scale) {
    auto result = parse("NUMERIC(*, 2)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::numeric {
            type::kind::numeric,
            type::decimal::flexible_precision,
            2,
    }));
}

TEST_F(sql_parser_type_test, decimal) {
    auto result = parse("DECIMAL");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::decimal {
            type::kind::decimal,
    }));
}

TEST_F(sql_parser_type_test, decimal_precision) {
    auto result = parse("DECIMAL(10)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::decimal {
            type::kind::decimal,
            10,
    }));
}

TEST_F(sql_parser_type_test, decimal_precision_scale) {
    auto result = parse("DECIMAL(10, 2)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::decimal {
            type::kind::decimal,
            10,
            2,
    }));
}

TEST_F(sql_parser_type_test, decimal_flexible_precision) {
    auto result = parse("DECIMAL(*)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::decimal {
            type::kind::decimal,
            type::decimal::flexible_precision
    }));
}

TEST_F(sql_parser_type_test, decimal_flexible_precision_scale) {
    auto result = parse("DECIMAL(*, 2)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::decimal {
            type::kind::decimal,
            type::decimal::flexible_precision,
            2,
    }));
}

TEST_F(sql_parser_type_test, integer) {
    auto result = parse("INTEGER");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::simple {
            type::kind::integer,
    }));
}

TEST_F(sql_parser_type_test, int) {
    auto result = parse("INT");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::simple {
            type::kind::integer,
    }));
}

TEST_F(sql_parser_type_test, tinyint) {
    auto result = parse("TINYINT");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::simple {
            type::kind::tiny_integer,
    }));
}

TEST_F(sql_parser_type_test, smallint) {
    auto result = parse("SMALLINT");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::simple {
            type::kind::small_integer,
    }));
}

TEST_F(sql_parser_type_test, bigint) {
    auto result = parse("BIGINT");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::simple {
            type::kind::big_integer,
    }));
}

TEST_F(sql_parser_type_test, int_length) {
    auto result = parse("INT(63)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::binary_numeric {
            type::kind::binary_integer,
            63,
    }));
}

TEST_F(sql_parser_type_test, float) {
    auto result = parse("FLOAT");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::simple {
            type::kind::float_,
    }));
}

TEST_F(sql_parser_type_test, real) {
    auto result = parse("REAL");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::simple {
            type::kind::real,
    }));
}

TEST_F(sql_parser_type_test, double) {
    auto result = parse("DOUBLE");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::simple {
            type::kind::double_precision,
    }));
}

TEST_F(sql_parser_type_test, double_precision) {
    auto result = parse("DOUBLE PRECISION");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::simple {
            type::kind::double_precision,
    }));
}

TEST_F(sql_parser_type_test, float_length) {
    auto result = parse("FLOAT(53)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::binary_numeric {
            type::kind::binary_float,
            53,
    }));
}

TEST_F(sql_parser_type_test, boolean) {
    auto result = parse("BOOLEAN");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::simple {
            type::kind::boolean,
    }));
}

TEST_F(sql_parser_type_test, date) {
    auto result = parse("DATE");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::simple {
            type::kind::date,
    }));
}

TEST_F(sql_parser_type_test, time) {
    auto result = parse("TIME");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::datetime {
            type::kind::time,
    }));
}

TEST_F(sql_parser_type_test, time_with_time_zone) {
    auto result = parse("TIME WITH TIME ZONE");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::datetime {
            type::kind::time,
            true,
    }));
}

TEST_F(sql_parser_type_test, time_without_time_zone) {
    auto result = parse("TIME WITHOUT TIME ZONE");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::datetime {
            type::kind::time,
            false,
    }));
}

TEST_F(sql_parser_type_test, timestamp) {
    auto result = parse("TIMESTAMP");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::datetime {
            type::kind::timestamp,
    }));
}

TEST_F(sql_parser_type_test, timestamp_with_time_zone) {
    auto result = parse("TIMESTAMP WITH TIME ZONE");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::datetime {
            type::kind::timestamp,
            true,
    }));
}

TEST_F(sql_parser_type_test, timestamp_without_time_zone) {
    auto result = parse("TIMESTAMP WITHOUT TIME ZONE");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::datetime {
            type::kind::timestamp,
            false,
    }));
}

TEST_F(sql_parser_type_test, interval) {
    auto result = parse("INTERVAL");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::interval {
    }));
}

TEST_F(sql_parser_type_test, DISABLED_interval_qualifier) {
    auto result = parse("INTERVAL YEAR TO DAY");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::interval {
    }));
}

TEST_F(sql_parser_type_test, row) {
    auto result = parse("ROW (a int)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::row {
            {
                    name::simple { "a" },
                    type::simple { type::kind::integer },
            },
    }));
}

TEST_F(sql_parser_type_test, row_multiple_fields) {
    auto result = parse("ROW (a int, b tinyint, c bigint)");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::row {
            {
                    name::simple { "a" },
                    type::simple { type::kind::integer },
            },
            {
                    name::simple { "b" },
                    type::simple { type::kind::tiny_integer },
            },
            {
                    name::simple { "c" },
                    type::simple { type::kind::big_integer },
            },
    }));
}

TEST_F(sql_parser_type_test, user_defined) {
    auto result = parse("t");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::user_defined {
            name::simple { "t" },
    }));
}

TEST_F(sql_parser_type_test, user_defined_chain_identifier) {
    auto result = parse("u.v.w");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::user_defined {
            name::qualified {
                    name::simple { "u" },
                    name::simple { "v" },
                    name::simple { "w" },
            },
    }));
}

TEST_F(sql_parser_type_test, collection) {
    auto result = parse("INT array[]");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::collection {
            type::simple {
                    type::kind::integer,
            },
            {},
    }));
}

TEST_F(sql_parser_type_test, collection_length) {
    auto result = parse("INT array[10]");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::collection {
            type::simple {
                    type::kind::integer,
            },
            10,
    }));
}

TEST_F(sql_parser_type_test, collection_flexible_length) {
    auto result = parse("INT array[*]");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::collection {
            type::simple {
                    type::kind::integer,
            },
            type::collection::flexible_length,
    }));
}

TEST_F(sql_parser_type_test, collection_simple) {
    auto result = parse("INT[]");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::collection {
            type::simple {
                    type::kind::integer,
            },
            {},
    }));
}

TEST_F(sql_parser_type_test, collection_simple_length) {
    auto result = parse("INT[10]");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::collection {
            type::simple {
                    type::kind::integer,
            },
            10,
    }));
}

TEST_F(sql_parser_type_test, collection_simple_flexible_length) {
    auto result = parse("INT[*]");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::collection {
            type::simple {
                    type::kind::integer,
            },
            type::collection::flexible_length,
    }));
}

TEST_F(sql_parser_type_test, collection_multiple) {
    auto result = parse("INT ARRAY[] ARRAY[]");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::collection {
            type::simple {
                    type::kind::integer,
            },
            std::nullopt,
            std::nullopt,
    }));
}

TEST_F(sql_parser_type_test, collection_multiple_length) {
    auto result = parse("INT ARRAY[10] ARRAY[20]");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::collection {
            type::simple {
                    type::kind::integer,
            },
            10,
            20,
    }));
}

TEST_F(sql_parser_type_test, collection_simple_multiple) {
    auto result = parse("INT[][]");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::collection {
            type::simple {
                    type::kind::integer,
            },
            std::nullopt,
            std::nullopt,
    }));
}

TEST_F(sql_parser_type_test, collection_simple_multiple_length) {
    auto result = parse("INT[10][20]");
    ASSERT_TRUE(result) << diagnostics(result);

    EXPECT_EQ(extract(result), (type::collection {
            type::simple{
                    type::kind::integer,
            },
            10,
            20,
    }));
}

} // namespace mizugaki::parser
