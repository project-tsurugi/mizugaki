#include <mizugaki/analyzer/details/analyze_literal.h>

#include <gtest/gtest.h>

#include <takatori/value/primitive.h>
#include <takatori/value/decimal.h>
#include <takatori/value/character.h>

#include <takatori/type/primitive.h>
#include <takatori/type/decimal.h>
#include <takatori/type/character.h>

#include <takatori/scalar/immediate.h>
#include <takatori/scalar/cast.h>

#include <mizugaki/ast/literal/numeric.h>
#include <mizugaki/ast/literal/string.h> // NOLINT
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
        EXPECT_FALSE(r);
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
}

TEST_F(analyze_literal_test, unsigned_mpint) {
    auto r = analyze_literal(
            context(),
            ast::literal::numeric {
                    ast::literal::kind::exact_numeric,
                    "1234567890123456789012345",
            });
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, (tscalar::immediate {
            tvalue::decimal { "1234567890123456789012345" },
            ttype::decimal { 25 },
    }));
}

TEST_F(analyze_literal_test, unsigned_decimal) {
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
}

TEST_F(analyze_literal_test, exact_numeric_decimal38_boundary_max) {
    options_.max_decimal_precision() = 38;
    options_.prefer_small_decimal_literals() = false;
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
                    ttype::decimal { {}, 0 },
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
                    ttype::decimal { {}, 0 },
            }));
}

TEST_F(analyze_literal_test, exact_numeric_decimal38_boundary_exponents) {
    options_.max_decimal_precision() = 38;
    options_.prefer_small_decimal_literals() = false;
    std::string s9x39;
    s9x39.resize(39, '9');
    for (int dot_position = 0; dot_position < static_cast<int>(s9x39.size()); ++dot_position) {
        auto unsigned_value = s9x39;
        unsigned_value[s9x39.size() - (dot_position + 1)] = '.';
        EXPECT_EQ(*parse_exact_numeric(ast::literal::sign::plus, unsigned_value),
                (tscalar::immediate {
                        tvalue::decimal {
                                tvalue::decimal::entity_type {
                                        +1,
                                        5421010862427522170ULL, // coef-hi
                                        687399551400673279ULL, // coef-lo
                                        -dot_position,
                                }
                        },
                        ttype::decimal { {}, 0 },
                }));
        EXPECT_EQ(*parse_exact_numeric(ast::literal::sign::minus, unsigned_value),
                (tscalar::immediate {
                        tvalue::decimal {
                                tvalue::decimal::entity_type {
                                        -1,
                                        5421010862427522170ULL, // coef-hi
                                        687399551400673279ULL, // coef-lo
                                        -dot_position,
                                }
                        },
                        ttype::decimal { {}, 0 },
                }));
    }
}

TEST_F(analyze_literal_test, exact_numeric_decimal38_boundary_over) {
    options_.max_decimal_precision() = 38;
    options_.prefer_small_decimal_literals() = false;
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
}

TEST_F(analyze_literal_test, character_string) {
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
}

TEST_F(analyze_literal_test, character_string_quote) {
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
}

TEST_F(analyze_literal_test, character_string_concat) {
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
}

TEST_F(analyze_literal_test, null) {
    options_.allow_context_independent_null() = false;
    auto r = analyze_literal(
            context(),
            ast::literal::null {},
            ttype::int4 {});
    ASSERT_TRUE(r);
    EXPECT_EQ(*r, (tscalar::immediate {
            tvalue::unknown { tvalue::unknown_kind::null },
            ttype::int4 {},
    }));
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
}

TEST_F(analyze_literal_test, null_wo_context) {
    options_.allow_context_independent_null() = false;
    auto r = analyze_literal(
            context(),
            ast::literal::null {});
    EXPECT_FALSE(r);
    EXPECT_TRUE(contains(context(), diagnostic_code::missing_context_of_null));
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
}

} // namespace mizugaki::analyzer::details
