#include <mizugaki/analyzer/details/analyze_literal.h>

#include <gtest/gtest.h>

#include <takatori/value/primitive.h>
#include <takatori/value/decimal.h>
#include <takatori/value/character.h>

#include <takatori/type/primitive.h>
#include <takatori/type/decimal.h>
#include <takatori/type/character.h>

#include <takatori/scalar/immediate.h>

#include <mizugaki/ast/literal/numeric.h>
#include <mizugaki/ast/literal/string.h> // NOLINT
#include <mizugaki/ast/literal/special.h>

#include "test_parent.h"

namespace mizugaki::analyzer::details {

using namespace ::mizugaki::analyzer::testing;

class analyze_literal_test : public test_parent {};

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

TEST_F(analyze_literal_test, dfault) {
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

TEST_F(analyze_literal_test, dfault_wo_context) {
    auto r = analyze_literal(
            context(),
            ast::literal::default_ {},
            {
                    ttype::int4 {},
            });
    EXPECT_FALSE(r);
    EXPECT_TRUE(contains(context(), diagnostic_code::missing_context_of_default_value));
}

} // namespace mizugaki::analyzer::details
