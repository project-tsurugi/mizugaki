#include <mizugaki/analyzer/details/analyze_literal.h>

#include <gtest/gtest.h>

#include <takatori/value/primitive.h>
#include <takatori/value/decimal.h>
#include <takatori/value/character.h>
#include <takatori/value/date.h>
#include <takatori/value/time_of_day.h>
#include <takatori/value/time_point.h>

#include <takatori/type/primitive.h>
#include <takatori/type/decimal.h>
#include <takatori/type/character.h>
#include <takatori/type/date.h>
#include <takatori/type/time_of_day.h>
#include <takatori/type/time_point.h>

#include <takatori/scalar/immediate.h>
#include <takatori/scalar/cast.h>

#include <mizugaki/ast/literal/numeric.h>
#include <mizugaki/ast/literal/string.h> // NOLINT
#include <mizugaki/ast/literal/datetime.h>
#include <mizugaki/ast/literal/special.h>

#include "test_parent.h"

namespace mizugaki::analyzer::details {

using namespace ::mizugaki::analyzer::testing;

class analyze_literal_test : public test_parent {
protected:
    std::unique_ptr<tscalar::expression> parse_exact_numeric(ast::literal::sign sign, std::string unsigned_value) {
        auto r = analyze_literal(
                context(),
                ast::literal::numeric {
                        ast::literal::kind::exact_numeric,
                        { sign },
                        { std::move(unsigned_value) },
                });
        if (!r || r->kind() != tscalar::immediate::tag) {
            ADD_FAILURE() << diagnostics();
            return std::make_unique<tscalar::immediate>(
                    tvalue::unknown {},
                    ttype::unknown {});
        }
        return r;
    }

    void invalid(ast::literal::literal const& literal) {
        auto r = analyze_literal(context(), literal);
        if (r) {
            ADD_FAILURE() << *r;
        }
        EXPECT_NE(count_error(), 0);
    }

    void invalid(sql_analyzer_code code, ast::literal::literal const& literal) {
        invalid(literal);
        EXPECT_TRUE(find_error(code)) << diagnostics();
    }
};

TEST_F(analyze_literal_test, unsigned_int1) {
    options_.prefer_small_integer_literals() = true;
    auto r = analyze_literal(
            context(),
            ast::literal::numeric {
                    ast::literal::kind::exact_numeric,
                    "1",
            });
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, (tscalar::immediate {
            tvalue::int4 { 1 },
            ttype::int1 {},
    }));
    expect_no_error();
}

TEST_F(analyze_literal_test, unsigned_int2) {
    options_.prefer_small_integer_literals() = true;
    auto r = analyze_literal(
            context(),
            ast::literal::numeric {
                    ast::literal::kind::exact_numeric,
                    "1234",
            });
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, (tscalar::immediate {
            tvalue::int4 { 1234 },
            ttype::int2 {},
    }));
    expect_no_error();
}

TEST_F(analyze_literal_test, unsigned_int4) {
    options_.prefer_small_integer_literals() = true;
    auto r = analyze_literal(
            context(),
            ast::literal::numeric {
                    ast::literal::kind::exact_numeric,
                    "1234567890",
            });
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, (tscalar::immediate {
            tvalue::int4 { 1234567890 },
            ttype::int4 {},
    }));
    expect_no_error();
}

TEST_F(analyze_literal_test, unsigned_int4_widening) {
    options_.prefer_small_integer_literals() = false;
    auto r = analyze_literal(
            context(),
            ast::literal::numeric {
                    ast::literal::kind::exact_numeric,
                    "1234567890",
            });
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, (tscalar::immediate {
            tvalue::int8 { 1234567890 },
            ttype::int8 {},
    }));
    expect_no_error();
}

TEST_F(analyze_literal_test, unsigned_int8) {
    auto r = analyze_literal(
            context(),
            ast::literal::numeric {
                    ast::literal::kind::exact_numeric,
                    "12345678901234567",
            });
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, (tscalar::immediate {
            tvalue::int8 { 12345678901234567LL },
            ttype::int8 {},
    }));
    expect_no_error();
}

TEST_F(analyze_literal_test, unsigned_mpint) {
    options_.prefer_small_decimal_literals() = true;
    auto r = analyze_literal(
            context(),
            ast::literal::numeric {
                    ast::literal::kind::exact_numeric,
                    "1234567890123456789012345",
            });
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, (tscalar::immediate {
            tvalue::decimal { "1234567890123456789012345" },
            ttype::decimal { 25, 0 },
    }));
    expect_no_error();
}

TEST_F(analyze_literal_test, unsigned_decimal) {
    options_.prefer_small_decimal_literals() = true;
    auto r = analyze_literal(
            context(),
            ast::literal::numeric {
                    ast::literal::kind::exact_numeric,
                    "1.4142156",
            });
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, (tscalar::immediate {
            tvalue::decimal { "1.4142156" },
            ttype::decimal { 8, 7 },
    }));
    expect_no_error();
}

TEST_F(analyze_literal_test, exact_numeric_int1_boundary) {
    options_.prefer_small_integer_literals() = true;
    EXPECT_EQ(*parse_exact_numeric(ast::literal::sign::plus, "0"),
            (tscalar::immediate {
                    tvalue::int4 { +0 },
                    ttype::int1 {},
            }));
    EXPECT_EQ(*parse_exact_numeric(ast::literal::sign::minus, "0"),
            (tscalar::immediate {
                    tvalue::int4 { -0 },
                    ttype::int1 {},
            }));
    EXPECT_EQ(*parse_exact_numeric(ast::literal::sign::plus, "127"),
            (tscalar::immediate {
                    tvalue::int4 { +127 },
                    ttype::int1 {},
            }));
    EXPECT_EQ(*parse_exact_numeric(ast::literal::sign::minus, "128"),
            (tscalar::immediate {
                    tvalue::int4 { -128 },
                    ttype::int1 {},
            }));
    expect_no_error();
}

TEST_F(analyze_literal_test, exact_numeric_int2_boundary) {
    options_.prefer_small_integer_literals() = true;
    EXPECT_EQ(*parse_exact_numeric(ast::literal::sign::plus, "128"),
            (tscalar::immediate {
                    tvalue::int4 { +128 },
                    ttype::int2 {},
            }));
    EXPECT_EQ(*parse_exact_numeric(ast::literal::sign::minus, "129"),
            (tscalar::immediate {
                    tvalue::int4 { -129 },
                    ttype::int2 {},
            }));
    EXPECT_EQ(*parse_exact_numeric(ast::literal::sign::plus, "32767"),
            (tscalar::immediate {
                    tvalue::int4 { +32767 },
                    ttype::int2 {},
            }));
    EXPECT_EQ(*parse_exact_numeric(ast::literal::sign::minus, "32768"),
            (tscalar::immediate {
                    tvalue::int4 { -32768 },
                    ttype::int2 {},
            }));
    expect_no_error();
}

TEST_F(analyze_literal_test, exact_numeric_int4_boundary) {
    options_.prefer_small_integer_literals() = true;
    EXPECT_EQ(*parse_exact_numeric(ast::literal::sign::plus, "32768"),
            (tscalar::immediate {
                    tvalue::int4 { +32768 },
                    ttype::int4 {},
            }));
    EXPECT_EQ(*parse_exact_numeric(ast::literal::sign::minus, "32769"),
            (tscalar::immediate {
                    tvalue::int4 { -32769 },
                    ttype::int4 {},
            }));
    EXPECT_EQ(*parse_exact_numeric(ast::literal::sign::plus, "2147483647"),
            (tscalar::immediate {
                    tvalue::int4 { +2147483647 },
                    ttype::int4 {},
            }));
    EXPECT_EQ(*parse_exact_numeric(ast::literal::sign::minus, "2147483648"),
            (tscalar::immediate {
                    tvalue::int4 { -2147483648 },
                    ttype::int4 {},
            }));
    expect_no_error();
}

TEST_F(analyze_literal_test, exact_numeric_int8_boundary) {
    EXPECT_EQ(*parse_exact_numeric(ast::literal::sign::plus, "2147483648"),
            (tscalar::immediate {
                    tvalue::int8 { +2147483648LL },
                    ttype::int8 {},
            }));
    EXPECT_EQ(*parse_exact_numeric(ast::literal::sign::minus, "2147483649"),
            (tscalar::immediate {
                    tvalue::int8 { -2147483649LL },
                    ttype::int8 {},
            }));
    EXPECT_EQ(*parse_exact_numeric(ast::literal::sign::plus, "9223372036854775807"),
            (tscalar::immediate {
                    tvalue::int8 { +9223372036854775807LL },
                    ttype::int8 {},
            }));
    EXPECT_EQ(*parse_exact_numeric(ast::literal::sign::minus, "9223372036854775808"),
            (tscalar::immediate {
                    tvalue::int8 { -9223372036854775807LL - 1LL },
                    ttype::int8 {},
            }));
    expect_no_error();
}

TEST_F(analyze_literal_test, exact_numeric_decimal38_boundary_sint8) {
    options_.max_decimal_precision() = 38;
    options_.prefer_small_decimal_literals() = false;
    EXPECT_EQ(*parse_exact_numeric(ast::literal::sign::plus, "9223372036854775808"),
            (tscalar::immediate {
                    tvalue::decimal {
                            tvalue::decimal::entity_type {
                                    +1,
                                    0ULL, // coef-hi
                                    9223372036854775808ULL, // coef-lo
                                    0,
                            }
                    },
                    ttype::decimal { {}, 0 },
            }));
    EXPECT_EQ(*parse_exact_numeric(ast::literal::sign::minus, "9223372036854775809"),
            (tscalar::immediate {
                    tvalue::decimal {
                            tvalue::decimal::entity_type {
                                    -1,
                                    0ULL, // coef-hi
                                    9223372036854775809ULL, // coef-lo
                                    0,
                            }
                    },
                    ttype::decimal { {}, 0 },
            }));
    expect_no_error();
}

TEST_F(analyze_literal_test, exact_numeric_decimal38_boundary_uint64) {
    options_.max_decimal_precision() = 38;
    options_.prefer_small_decimal_literals() = false;
    EXPECT_EQ(*parse_exact_numeric(ast::literal::sign::plus, "18446744073709551615"),
            (tscalar::immediate {
                    tvalue::decimal {
                            tvalue::decimal::entity_type {
                                    +1,
                                    0ULL, // coef-hi
                                    18446744073709551615ULL, // coef-lo
                                    0,
                            }
                    },
                    ttype::decimal { {}, 0 },
            }));
    EXPECT_EQ(*parse_exact_numeric(ast::literal::sign::minus, "18446744073709551615"),
            (tscalar::immediate {
                    tvalue::decimal {
                            tvalue::decimal::entity_type {
                                    -1,
                                    0ULL, // coef-hi
                                    18446744073709551615ULL, // coef-lo
                                    0,
                            }
                    },
                    ttype::decimal { {}, 0 },
            }));
    EXPECT_EQ(*parse_exact_numeric(ast::literal::sign::plus, "18446744073709551616"),
            (tscalar::immediate {
                    tvalue::decimal {
                            tvalue::decimal::entity_type {
                                    +1,
                                    1ULL, // coef-hi
                                    0ULL, // coef-lo
                                    0,
                            }
                    },
                    ttype::decimal { {}, 0 },
            }));
    EXPECT_EQ(*parse_exact_numeric(ast::literal::sign::minus, "18446744073709551616"),
            (tscalar::immediate {
                    tvalue::decimal {
                            tvalue::decimal::entity_type {
                                    -1,
                                    1ULL, // coef-hi
                                    0ULL, // coef-lo
                                    0,
                            }
                    },
                    ttype::decimal { {}, 0 },
            }));
    expect_no_error();
}

TEST_F(analyze_literal_test, exact_numeric_decimal38_boundary_max) {
    options_.max_decimal_precision() = 38;
    options_.prefer_small_decimal_literals() = true;
    std::string s9x38;
    s9x38.resize(38, '9');

    EXPECT_EQ(*parse_exact_numeric(ast::literal::sign::plus, s9x38),
            (tscalar::immediate {
                    tvalue::decimal {
                            tvalue::decimal::entity_type {
                                    +1,
                                    5421010862427522170ULL, // coef-hi
                                    687399551400673279ULL, // coef-lo
                                    0,
                            }
                    },
                    ttype::decimal { 38, 0 },
            }));
    EXPECT_EQ(*parse_exact_numeric(ast::literal::sign::minus, s9x38),
            (tscalar::immediate {
                    tvalue::decimal {
                            tvalue::decimal::entity_type {
                                    -1,
                                    5421010862427522170ULL, // coef-hi
                                    687399551400673279ULL, // coef-lo
                                    0,
                            }
                    },
                    ttype::decimal { 38, 0 },
            }));
    expect_no_error();
}

TEST_F(analyze_literal_test, exact_numeric_decimal38_boundary_exponents) {
    options_.max_decimal_precision() = 38;
    options_.prefer_small_decimal_literals() = true;
    std::string s9x39;
    s9x39.resize(39, '9');
    for (int scale = 0; scale < static_cast<int>(s9x39.size()); ++scale) {
        auto unsigned_value = s9x39;
        unsigned_value[s9x39.size() - (scale + 1)] = '.';
        EXPECT_EQ(*parse_exact_numeric(ast::literal::sign::plus, unsigned_value),
                (tscalar::immediate {
                        tvalue::decimal {
                                tvalue::decimal::entity_type {
                                        +1,
                                        5421010862427522170ULL, // coef-hi
                                        687399551400673279ULL, // coef-lo
                                        -scale,
                                }
                        },
                        ttype::decimal { 38, scale },
                }));
        EXPECT_EQ(*parse_exact_numeric(ast::literal::sign::minus, unsigned_value),
                (tscalar::immediate {
                        tvalue::decimal {
                                tvalue::decimal::entity_type {
                                        -1,
                                        5421010862427522170ULL, // coef-hi
                                        687399551400673279ULL, // coef-lo
                                        -scale,
                                }
                        },
                        ttype::decimal { 38, scale },
                }));
    }
    expect_no_error();
}

TEST_F(analyze_literal_test, exact_numeric_decimal38_boundary_min) {
    options_.max_decimal_precision() = 38;
    options_.prefer_small_decimal_literals() = true;
    std::string p38;
    p38.resize(38, '0');
    p38[37] = '1'; // 0{37}1
    EXPECT_EQ(*parse_exact_numeric(ast::literal::sign::plus, "." + p38),
            (tscalar::immediate {
                    tvalue::decimal {
                            tvalue::decimal::entity_type {
                                    +1,
                                    0ULL, // coef-hi
                                    1ULL, // coef-lo
                                    -38,
                            }
                    },
                    ttype::decimal { 38, 38 },
            }));
    EXPECT_EQ(*parse_exact_numeric(ast::literal::sign::minus, "." + p38),
            (tscalar::immediate {
                    tvalue::decimal {
                            tvalue::decimal::entity_type {
                                    -1,
                                    0ULL, // coef-hi
                                    1ULL, // coef-lo
                                    -38,
                            }
                    },
                    ttype::decimal { 38, 38 },
            }));
    EXPECT_EQ(*parse_exact_numeric(ast::literal::sign::plus, "0." + p38),
            (tscalar::immediate {
                    tvalue::decimal {
                            tvalue::decimal::entity_type {
                                    +1,
                                    0ULL, // coef-hi
                                    1ULL, // coef-lo
                                    -38,
                            }
                    },
                    ttype::decimal { 38, 38 },
            }));
    EXPECT_EQ(*parse_exact_numeric(ast::literal::sign::minus, "0." + p38),
            (tscalar::immediate {
                    tvalue::decimal {
                            tvalue::decimal::entity_type {
                                    -1,
                                    0ULL, // coef-hi
                                    1ULL, // coef-lo
                                    -38,
                            }
                    },
                    ttype::decimal { 38, 38 },
            }));
    expect_no_error();
}

TEST_F(analyze_literal_test, exact_numeric_decimal38_boundary_leading_zeros) {
    options_.max_decimal_precision() = 38;
    options_.prefer_small_decimal_literals() = true;
    std::string p38;
    p38.resize(38, '0'); // 0{38}
    EXPECT_EQ(*parse_exact_numeric(ast::literal::sign::plus, "." + p38),
            (tscalar::immediate {
                    tvalue::decimal {
                            tvalue::decimal::entity_type {
                                    +1,
                                    0ULL, // coef-hi
                                    0ULL, // coef-lo
                                    -38,
                            }
                    },
                    ttype::decimal { 38, 38 },
            }));
    EXPECT_EQ(*parse_exact_numeric(ast::literal::sign::minus, "." + p38),
            (tscalar::immediate {
                    tvalue::decimal {
                            tvalue::decimal::entity_type {
                                    -1,
                                    0ULL, // coef-hi
                                    0ULL, // coef-lo
                                    -38,
                            }
                    },
                    ttype::decimal { 38, 38 },
            }));
    expect_no_error();
}

TEST_F(analyze_literal_test, exact_numeric_decimal38_boundary_one_leading_zeros) {
    options_.max_decimal_precision() = 38;
    options_.prefer_small_decimal_literals() = true;
    std::string p37;
    p37.resize(37, '0'); // 0{37}
    EXPECT_EQ(*parse_exact_numeric(ast::literal::sign::plus, "1." + p37),
            (tscalar::immediate {
                    tvalue::decimal {
                            tvalue::decimal::entity_type {
                                    +1,
                                    542101086242752217ULL, // coef-hi
                                    68739955140067328ULL, // coef-lo
                                    -37,
                            }
                    },
                    ttype::decimal { 38, 37 },
            }));
    EXPECT_EQ(*parse_exact_numeric(ast::literal::sign::minus, "1." + p37),
            (tscalar::immediate {
                    tvalue::decimal {
                            tvalue::decimal::entity_type {
                                    -1,
                                    542101086242752217ULL, // coef-hi
                                    68739955140067328ULL, // coef-lo
                                    -37,
                            }
                    },
                    ttype::decimal { 38, 37 },
            }));
    expect_no_error();
}

TEST_F(analyze_literal_test, exact_numeric_decimal38_boundary_overflow) {
    options_.max_decimal_precision() = 38;
    std::string p39;
    p39.resize(39, '0');
    p39[0] = '1'; // 10{38}
    invalid(sql_analyzer_code::unsupported_decimal_value, ast::literal::numeric {
            ast::literal::kind::exact_numeric,
            {},
            { p39 },
    });
    invalid(sql_analyzer_code::unsupported_decimal_value, ast::literal::numeric {
            ast::literal::kind::exact_numeric,
            ast::literal::sign::minus,
            { p39 },
    });
}

TEST_F(analyze_literal_test, exact_numeric_decimal38_boundary_rounding) {
    options_.max_decimal_precision() = 38;
    std::string p38;
    p38.resize(38, '0');
    p38[0] = '1'; // 10^37
    invalid(sql_analyzer_code::unsupported_decimal_value, ast::literal::numeric {
            ast::literal::kind::exact_numeric,
            {},
            { p38 + ".0" },
    });
}

TEST_F(analyze_literal_test, exact_numeric_decimal38_boundary_undeflow) {
    options_.max_decimal_precision() = 38;
    std::string p39;
    p39.resize(39, '0');
    p39[38] = '1'; // 0{38}1
    invalid(sql_analyzer_code::unsupported_decimal_value, ast::literal::numeric {
            ast::literal::kind::exact_numeric,
            {},
            { "." + p39 },
    });
}

TEST_F(analyze_literal_test, unsigned_approx_numeric) {
    auto r = analyze_literal(
            context(),
            ast::literal::numeric {
                    ast::literal::kind::approximate_numeric,
                    "1.25E0",
            });
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, (tscalar::immediate {
            tvalue::float8 { 1.25 },
            ttype::float8 {},
    }));
    expect_no_error();
}

TEST_F(analyze_literal_test, signed_approx_numeric) {
    auto r = analyze_literal(
            context(),
            ast::literal::numeric {
                    ast::literal::kind::approximate_numeric,
                    { ast::literal::sign::minus },
                    { "125E-2" },
            });
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, (tscalar::immediate {
            tvalue::float8 { -1.25 },
            ttype::float8 {},
    }));
    expect_no_error();
}

TEST_F(analyze_literal_test, character_string) {
    options_.prefer_small_character_literals() = true;
    auto r = analyze_literal(
            context(),
            ast::literal::string {
                    ast::literal::kind::character_string,
                    "'Hello, world!'",
            });
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, (tscalar::immediate {
            tvalue::character { "Hello, world!" },
            ttype::character { ttype::varying, 13 },
    }));
    expect_no_error();
}

TEST_F(analyze_literal_test, character_string_quote) {
    options_.prefer_small_character_literals() = true;
    auto r = analyze_literal(
            context(),
            ast::literal::string {
                    ast::literal::kind::character_string,
                    "'Hello, ''world''!'",
            });
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, (tscalar::immediate {
            tvalue::character { "Hello, 'world'!" },
            ttype::character { ttype::varying, 15 },
    }));
    expect_no_error();
}

TEST_F(analyze_literal_test, character_string_concat) {
    options_.prefer_small_character_literals() = true;
    auto r = analyze_literal(
            context(),
            ast::literal::string {
                    ast::literal::kind::character_string,
                    "'Hello'",
                    "', '",
                    "'world'",
                    "'!'",
            });
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, (tscalar::immediate {
            tvalue::character { "Hello, world!" },
            ttype::character { ttype::varying, 13 },
    }));
    expect_no_error();
}

TEST_F(analyze_literal_test, date) {
    auto r = analyze_literal(
            context(),
            ast::literal::datetime {
                    ast::literal::kind::date,
                    "'2001-02-03'",
            });
    ASSERT_TRUE(r) << diagnostics();
    EXPECT_EQ(*r, (tscalar::immediate {
            tvalue::date { 2001, 2, 3 },
            ttype::date {},
    }));
    expect_no_error();
}

TEST_F(analyze_literal_test, date_invalid) {
    invalid(sql_analyzer_code::unsupported_string_value, ast::literal::datetime {
            ast::literal::kind::date,
            "'1-2-0'",
    });
}

TEST_F(analyze_literal_test, time) {
    auto r = analyze_literal(
            context(),
            ast::literal::datetime {
                    ast::literal::kind::time,
                    "'12:34:56'",
            });
    ASSERT_TRUE(r) << diagnostics();
    EXPECT_EQ(*r, (tscalar::immediate {
            tvalue::time_of_day { 12, 34, 56 },
            ttype::time_of_day {},
    }));
    expect_no_error();
}

TEST_F(analyze_literal_test, time_invalid) {
    invalid(sql_analyzer_code::unsupported_string_value, ast::literal::datetime {
            ast::literal::kind::time,
            "'24:00:00'",
    });
}

TEST_F(analyze_literal_test, timestamp) {
    auto r = analyze_literal(
            context(),
            ast::literal::datetime {
                    ast::literal::kind::timestamp,
                    "'1970-1-2 12:34:56'",
            });
    ASSERT_TRUE(r) << diagnostics();
    EXPECT_EQ(*r, (tscalar::immediate {
            tvalue::time_point { 1970, 1, 2, 12, 34, 56 },
            ttype::time_point {},
    }));
    expect_no_error();
}

TEST_F(analyze_literal_test, timestamp_invalid) {
    invalid(sql_analyzer_code::unsupported_string_value, ast::literal::datetime {
            ast::literal::kind::timestamp,
            "'1970-8-32 0:0:0'",
    });
    invalid(sql_analyzer_code::unsupported_string_value, ast::literal::datetime {
            ast::literal::kind::timestamp,
            "'1970-1-2 0:0:0Z'",
    });
}

TEST_F(analyze_literal_test, timestamp_with_time_zone) {
    auto r = analyze_literal(
            context(),
            ast::literal::datetime {
                    ast::literal::kind::timestamp_with_time_zone,
                    "'1970-1-1 00:00:00+09:00'",
            });
    ASSERT_TRUE(r) << diagnostics();
    EXPECT_EQ(*r, (tscalar::immediate {
            tvalue::time_point { 1969, 12, 31, 15, 0, 0 },
            ttype::time_point { ttype::with_time_zone },
    }));
    expect_no_error();
}

TEST_F(analyze_literal_test, timestamp_with_time_zone_z) {
    auto r = analyze_literal(
            context(),
            ast::literal::datetime {
                    ast::literal::kind::timestamp_with_time_zone,
                    "'1970-1-1 00:00:00Z'",
            });
    ASSERT_TRUE(r) << diagnostics();
    EXPECT_EQ(*r, (tscalar::immediate {
            tvalue::time_point { 1970, 1, 1, 0, 0, 0 },
            ttype::time_point { ttype::with_time_zone },
    }));
    expect_no_error();
}

TEST_F(analyze_literal_test, timestamp_with_time_zone_default) {
    options_.system_zone_offset() = sql_analyzer_options::zone_offset_type { -9 * 60 };
    auto r = analyze_literal(
            context(),
            ast::literal::datetime {
                    ast::literal::kind::timestamp_with_time_zone,
                    "'1970-1-1 00:00:00'",
            });
    ASSERT_TRUE(r) << diagnostics();
    EXPECT_EQ(*r, (tscalar::immediate {
            tvalue::time_point { 1970, 1, 1, 9, 0, 0 },
            ttype::time_point { ttype::with_time_zone },
    }));
    expect_no_error();
}

TEST_F(analyze_literal_test, timestamp_with_time_zone_invalid) {
    invalid(sql_analyzer_code::unsupported_string_value, ast::literal::datetime {
            ast::literal::kind::timestamp_with_time_zone,
            "'0:0:0'",
    });
}

TEST_F(analyze_literal_test, null) {
    options_.allow_context_independent_null() = false;
    auto r = analyze_literal(
            context(),
            ast::literal::null {},
            ttype::int4 {});
    ASSERT_TRUE(r) << diagnostics();
    EXPECT_EQ(*r, (tscalar::immediate {
            tvalue::unknown { tvalue::unknown_kind::null },
            ttype::int4 {},
    }));
    expect_no_error();
}

TEST_F(analyze_literal_test, null_allow_independent) {
    options_.allow_context_independent_null() = true;
    auto r = analyze_literal(
            context(),
            ast::literal::null {});
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, (tscalar::immediate {
            tvalue::unknown { tvalue::unknown_kind::null },
            ttype::unknown {},
    }));
    expect_no_error();
}

TEST_F(analyze_literal_test, null_wo_context) {
    options_.allow_context_independent_null() = false;
    invalid(diagnostic_code::missing_context_of_null, ast::literal::null {});
}

TEST_F(analyze_literal_test, default) {
    auto r = analyze_literal(
            context(),
            ast::literal::default_ {},
            {
                    ttype::int4 {},
                    tvalue::int4 { 1 },
            });
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, (tscalar::immediate {
            tvalue::int4 { 1 },
            ttype::int4 {},
    }));
    expect_no_error();
}

TEST_F(analyze_literal_test, default_wo_context) {
    auto r = analyze_literal(
            context(),
            ast::literal::default_ {},
            {
                    ttype::int4 {},
            });
    EXPECT_FALSE(r);
    EXPECT_TRUE(contains(context(), diagnostic_code::missing_context_of_default_value));
}

TEST_F(analyze_literal_test, conversion_by_context_enabled) {
    options_.cast_literals_in_context() = true;
    auto r = analyze_literal(
            context(),
            ast::literal::string {
                    ast::literal::kind::character_string,
                    "'1'",
            },
            {
                    ttype::int8 {},
            });
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, (tscalar::cast {
            ttype::int8 {},
            tscalar::cast_loss_policy::error,
            tscalar::immediate {
                    tvalue::character { "1" },
                    ttype::character { ttype::varying },
            }
    }));
    expect_no_error();
}

TEST_F(analyze_literal_test, conversion_by_context_disabled) {
    options_.cast_literals_in_context() = false;
    auto r = analyze_literal(
            context(),
            ast::literal::string {
                    ast::literal::kind::character_string,
                    "'1'",
            },
            {
                    ttype::int8 {},
            });
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, (tscalar::immediate {
            tvalue::character { "1" },
            ttype::character { ttype::varying },
    }));
    expect_no_error();
}

} // namespace mizugaki::analyzer::details
