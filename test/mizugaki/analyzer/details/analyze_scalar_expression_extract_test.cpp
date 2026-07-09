#include <mizugaki/analyzer/details/analyze_scalar_expression.h>

#include <gtest/gtest.h>

#include <takatori/type/primitive.h>
#include <takatori/type/character.h>
#include <takatori/type/date.h>
#include <takatori/type/time_of_day.h>
#include <takatori/type/time_point.h>

#include <takatori/value/primitive.h>
#include <takatori/value/date.h>
#include <takatori/value/time_of_day.h>
#include <takatori/value/time_point.h>

#include <takatori/scalar/function_call.h>

#include <takatori/datetime/conversion.h>

#include <yugawara/binding/factory.h>

#include <yugawara/function/declaration.h>

#include <mizugaki/ast/literal/datetime.h>
#include <mizugaki/ast/type/character_string.h>
#include <mizugaki/ast/scalar/function_invocation.h>

#include "test_parent.h"
#include "mizugaki/ast/scalar/extract_expression.h"

namespace mizugaki::analyzer::details {

using namespace ::mizugaki::analyzer::testing;

class analyze_scalar_expression_extract_test : public test_parent {
protected:
    query_scope scope {};

    ::takatori::descriptor::function descriptor(std::shared_ptr<::yugawara::function::declaration const> decl) {
        return ::yugawara::binding::factory {}(std::move(decl));
    }

    void invalid(ast::scalar::expression const& expression) {
        auto r = analyze_scalar_expression(
                context(),
                expression,
                scope,
                {});
        EXPECT_FALSE(r) << diagnostics();
        EXPECT_NE(count_error(), 0);
    }

    void invalid(sql_analyzer_code code, ast::scalar::expression const& expression) {
        invalid(expression);
        EXPECT_TRUE(find_error(code));
    }

    ast::scalar::literal_expression value(ast::literal::kind kind, std::string_view value) {
        std::string str {};
        str.reserve(value.size() + 2);
        str.append("'");
        str.append(value);
        str.append("'");
        return ast::scalar::literal_expression {
                ast::literal::datetime {
                        kind,
                        std::move(str),
                },
        };
    }

    tscalar::immediate timestamp(std::string_view value, bool tz = false) {
        auto result = ::takatori::datetime::parse_datetime(value);
        if (!result) {
            throw std::invalid_argument("invalid timestamp value");
        }
        auto date = result->date;
        auto time = result->time;
        return tscalar::immediate {
                tvalue::time_point {
                        static_cast<std::int32_t>(date.year),
                        date.month,
                        date.day,
                        time.hour,
                        time.minute,
                        time.second,
                        time.subsecond
                },
                ttype::time_point {
                        ttype::with_time_zone_t { tz }
                },
        };
    }

    tscalar::immediate date(std::string_view value) {
        auto result = ::takatori::datetime::parse_date(value);
        if (!result) {
            throw std::invalid_argument("invalid timestamp value");
        }
        auto date = *result;
        return tscalar::immediate {
                tvalue::date {
                        date.year,
                        date.month,
                        date.day,
                },
                ttype::date {},
        };
    }

    tscalar::immediate time(std::string_view value) {
        auto result = ::takatori::datetime::parse_time(value);
        if (!result) {
            throw std::invalid_argument("invalid timestamp value");
        }
        auto time = *result;
        return tscalar::immediate {
                tvalue::time_of_day {
                        time.hour,
                        time.minute,
                        time.second,
                        time.subsecond,
                },
                ttype::time_of_day {},
        };
    }

    tscalar::immediate precision(std::int32_t value) {
        return tscalar::immediate {
                tvalue::int4 { value },
                ttype::int4 {},
        };
    }

    ast::scalar::literal_expression erroneous_expression() {
        return literal(string("INVALID"));
    }

    ast::type::character_string erroneous_type() {
        return ast::type::character_string { ast::type::kind::character_varying, 0 };
    }
};

TEST_F(analyze_scalar_expression_extract_test, year_timestamp) {
    auto func = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "extract_year",
            ttype::int4 {},
            {
                    ttype::time_point {},
            },
    });
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::extract_expression {
                    ast::scalar::extract_field_kind::year,
                    value(ast::literal::kind::timestamp, "2001-02-03 04:05:06.789"),
            },
            scope,
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_FALSE(r.saw_aggregate());
    EXPECT_EQ(*r, (tscalar::function_call {
            descriptor(func),
            {
                    timestamp("2001-02-03 04:05:06.789"),
            },
    }));
}

TEST_F(analyze_scalar_expression_extract_test, month_timestamp) {
    auto func = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "extract_month",
            ttype::int4 {},
            {
                    ttype::time_point {},
            },
    });
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::extract_expression {
                    ast::scalar::extract_field_kind::month,
                    value(ast::literal::kind::timestamp, "2001-02-03 04:05:06.789"),
            },
            scope,
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_FALSE(r.saw_aggregate());
    EXPECT_EQ(*r, (tscalar::function_call {
            descriptor(func),
            {
                    timestamp("2001-02-03 04:05:06.789"),
            },
    }));
}

TEST_F(analyze_scalar_expression_extract_test, day_timestamp) {
    auto func = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "extract_day",
            ttype::int4 {},
            {
                    ttype::time_point {},
            },
    });
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::extract_expression {
                    ast::scalar::extract_field_kind::day,
                    value(ast::literal::kind::timestamp, "2001-02-03 04:05:06.789"),
            },
            scope,
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_FALSE(r.saw_aggregate());
    EXPECT_EQ(*r, (tscalar::function_call {
            descriptor(func),
            {
                    timestamp("2001-02-03 04:05:06.789"),
            },
    }));
}

TEST_F(analyze_scalar_expression_extract_test, hour_timestamp) {
    auto func = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "extract_hour",
            ttype::int4 {},
            {
                    ttype::time_point {},
            },
    });
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::extract_expression {
                    ast::scalar::extract_field_kind::hour,
                    value(ast::literal::kind::timestamp, "2001-02-03 04:05:06.789"),
            },
            scope,
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_FALSE(r.saw_aggregate());
    EXPECT_EQ(*r, (tscalar::function_call {
            descriptor(func),
            {
                    timestamp("2001-02-03 04:05:06.789"),
            },
    }));
}

TEST_F(analyze_scalar_expression_extract_test, minute_timestamp) {
    auto func = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "extract_minute",
            ttype::int4 {},
            {
                    ttype::time_point {},
            },
    });
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::extract_expression {
                    ast::scalar::extract_field_kind::minute,
                    value(ast::literal::kind::timestamp, "2001-02-03 04:05:06.789"),
            },
            scope,
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_FALSE(r.saw_aggregate());
    EXPECT_EQ(*r, (tscalar::function_call {
            descriptor(func),
            {
                    timestamp("2001-02-03 04:05:06.789"),
            },
    }));
}

TEST_F(analyze_scalar_expression_extract_test, second_timestamp) {
    auto func = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "extract_second",
            ttype::int4 {},
            {
                    ttype::time_point {},
                    ttype::int4 {},
            },
    });
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::extract_expression {
                    ast::scalar::extract_field_kind::second,
                    value(ast::literal::kind::timestamp, "2001-02-03 04:05:06.789"),
            },
            scope,
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_FALSE(r.saw_aggregate());
    EXPECT_EQ(*r, (tscalar::function_call {
            descriptor(func),
            {
                    timestamp("2001-02-03 04:05:06.789"),
                    precision(9)
            },
    }));
}

TEST_F(analyze_scalar_expression_extract_test, second_precision_timestamp) {
    auto func = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "extract_second",
            ttype::int4 {},
            {
                    ttype::time_point {},
                    ttype::int4 {},
            },
    });
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::extract_expression {
                    ast::scalar::extract_field_kind::second,
                    6,
                    value(ast::literal::kind::timestamp, "2001-02-03 04:05:06.789"),
            },
            scope,
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_FALSE(r.saw_aggregate());
    EXPECT_EQ(*r, (tscalar::function_call {
            descriptor(func),
            {
                    timestamp("2001-02-03 04:05:06.789"),
                    precision(6)
            },
    }));
}

TEST_F(analyze_scalar_expression_extract_test, timezone_hour_timestamp) {
    auto func = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "extract_timezone_hour",
            ttype::int4 {},
            {
                    ttype::time_point {},
            },
    });
    invalid(sql_analyzer_code::function_not_found, ast::scalar::extract_expression {
            ast::scalar::extract_field_kind::timezone_hour,
            value(ast::literal::kind::timestamp, "2001-02-03 04:05:06.789"),
    });
}

TEST_F(analyze_scalar_expression_extract_test, timezone_minute_timestamp) {
    auto func = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "extract_timezone_minute",
            ttype::int4 {},
            {
                    ttype::time_point {},
            },
    });
    invalid(sql_analyzer_code::function_not_found, ast::scalar::extract_expression {
            ast::scalar::extract_field_kind::timezone_hour,
            value(ast::literal::kind::timestamp, "2001-02-03 04:05:06.789"),
    });
}

TEST_F(analyze_scalar_expression_extract_test, year_timestamptz) {
    auto func = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "extract_year",
            ttype::int4 {},
            {
                    ttype::time_point { ttype::with_time_zone },
            },
    });
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::extract_expression {
                    ast::scalar::extract_field_kind::year,
                    value(ast::literal::kind::timestamp_with_time_zone, "2001-02-03 04:05:06.789"),
            },
            scope,
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_FALSE(r.saw_aggregate());
    EXPECT_EQ(*r, (tscalar::function_call {
            descriptor(func),
            {
                    timestamp("2001-02-03 04:05:06.789", true),
            },
    }));
}

TEST_F(analyze_scalar_expression_extract_test, month_timestamptz) {
    auto func = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "extract_month",
            ttype::int4 {},
            {
                    ttype::time_point { ttype::with_time_zone },
            },
    });
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::extract_expression {
                    ast::scalar::extract_field_kind::month,
                    value(ast::literal::kind::timestamp_with_time_zone, "2001-02-03 04:05:06.789"),
            },
            scope,
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_FALSE(r.saw_aggregate());
    EXPECT_EQ(*r, (tscalar::function_call {
            descriptor(func),
            {
                    timestamp("2001-02-03 04:05:06.789", true),
            },
    }));
}

TEST_F(analyze_scalar_expression_extract_test, day_timestamptz) {
    auto func = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "extract_day",
            ttype::int4 {},
            {
                    ttype::time_point { ttype::with_time_zone },
            },
    });
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::extract_expression {
                    ast::scalar::extract_field_kind::day,
                    value(ast::literal::kind::timestamp_with_time_zone, "2001-02-03 04:05:06.789"),
            },
            scope,
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_FALSE(r.saw_aggregate());
    EXPECT_EQ(*r, (tscalar::function_call {
            descriptor(func),
            {
                    timestamp("2001-02-03 04:05:06.789", true),
            },
    }));
}

TEST_F(analyze_scalar_expression_extract_test, hour_timestamptz) {
    auto func = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "extract_hour",
            ttype::int4 {},
            {
                    ttype::time_point { ttype::with_time_zone },
            },
    });
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::extract_expression {
                    ast::scalar::extract_field_kind::hour,
                    value(ast::literal::kind::timestamp_with_time_zone, "2001-02-03 04:05:06.789"),
            },
            scope,
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_FALSE(r.saw_aggregate());
    EXPECT_EQ(*r, (tscalar::function_call {
            descriptor(func),
            {
                    timestamp("2001-02-03 04:05:06.789", true),
            },
    }));
}

TEST_F(analyze_scalar_expression_extract_test, minute_timestamptz) {
    auto func = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "extract_minute",
            ttype::int4 {},
            {
                    ttype::time_point { ttype::with_time_zone },
            },
    });
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::extract_expression {
                    ast::scalar::extract_field_kind::minute,
                    value(ast::literal::kind::timestamp_with_time_zone, "2001-02-03 04:05:06.789"),
            },
            scope,
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_FALSE(r.saw_aggregate());
    EXPECT_EQ(*r, (tscalar::function_call {
            descriptor(func),
            {
                    timestamp("2001-02-03 04:05:06.789", true),
            },
    }));
}

TEST_F(analyze_scalar_expression_extract_test, second_timestamptz) {
    auto func = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "extract_second",
            ttype::int4 {},
            {
                    ttype::time_point { ttype::with_time_zone },
                    ttype::int4 {},
            },
    });
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::extract_expression {
                    ast::scalar::extract_field_kind::second,
                    value(ast::literal::kind::timestamp_with_time_zone, "2001-02-03 04:05:06.789"),
            },
            scope,
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_FALSE(r.saw_aggregate());
    EXPECT_EQ(*r, (tscalar::function_call {
            descriptor(func),
            {
                    timestamp("2001-02-03 04:05:06.789", true),
                    precision(9)
            },
    }));
}

TEST_F(analyze_scalar_expression_extract_test, timezone_hour_timestamptz) {
    auto func = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "extract_timezone_hour",
            ttype::int4 {},
            {
                    ttype::time_point { ttype::with_time_zone },
            },
    });
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::extract_expression {
                    ast::scalar::extract_field_kind::timezone_hour,
                    value(ast::literal::kind::timestamp_with_time_zone, "2001-02-03 04:05:06.789"),
            },
            scope,
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_FALSE(r.saw_aggregate());
    EXPECT_EQ(*r, (tscalar::function_call {
            descriptor(func),
            {
                    timestamp("2001-02-03 04:05:06.789", true),
            },
    }));
}

TEST_F(analyze_scalar_expression_extract_test, timezone_minute_timestamptz) {
    auto func = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "extract_timezone_minute",
            ttype::int4 {},
            {
                    ttype::time_point { ttype::with_time_zone },
            },
    });
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::extract_expression {
                    ast::scalar::extract_field_kind::timezone_minute,
                    value(ast::literal::kind::timestamp_with_time_zone, "2001-02-03 04:05:06.789"),
            },
            scope,
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_FALSE(r.saw_aggregate());
    EXPECT_EQ(*r, (tscalar::function_call {
            descriptor(func),
            {
                    timestamp("2001-02-03 04:05:06.789", true),
            },
    }));
}



TEST_F(analyze_scalar_expression_extract_test, year_date) {
    auto func = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "extract_year",
            ttype::int4 {},
            {
                    ttype::date {},
            },
    });
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::extract_expression {
                    ast::scalar::extract_field_kind::year,
                    value(ast::literal::kind::date, "2001-02-03"),
            },
            scope,
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_FALSE(r.saw_aggregate());
    EXPECT_EQ(*r, (tscalar::function_call {
            descriptor(func),
            {
                    date("2001-02-03"),
            },
    }));
}

TEST_F(analyze_scalar_expression_extract_test, month_date) {
    auto func = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "extract_month",
            ttype::int4 {},
            {
                    ttype::date {},
            },
    });
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::extract_expression {
                    ast::scalar::extract_field_kind::month,
                    value(ast::literal::kind::date, "2001-02-03"),
            },
            scope,
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_FALSE(r.saw_aggregate());
    EXPECT_EQ(*r, (tscalar::function_call {
            descriptor(func),
            {
                    date("2001-02-03"),
            },
    }));
}

TEST_F(analyze_scalar_expression_extract_test, day_date) {
    auto func = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "extract_day",
            ttype::int4 {},
            {
                    ttype::date {},
            },
    });
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::extract_expression {
                    ast::scalar::extract_field_kind::day,
                    value(ast::literal::kind::date, "2001-02-03"),
            },
            scope,
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_FALSE(r.saw_aggregate());
    EXPECT_EQ(*r, (tscalar::function_call {
            descriptor(func),
            {
                    date("2001-02-03"),
            },
    }));
}

TEST_F(analyze_scalar_expression_extract_test, hour_date) {
    auto func = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "extract_hour",
            ttype::int4 {},
            {
                    ttype::date {},
            },
    });
    invalid(sql_analyzer_code::function_not_found, ast::scalar::extract_expression {
            ast::scalar::extract_field_kind::hour,
            value(ast::literal::kind::date, "2001-02-03"),
    });
}

TEST_F(analyze_scalar_expression_extract_test, hour_time) {
    auto func = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "extract_hour",
            ttype::int4 {},
            {
                    ttype::time_of_day {},
            },
    });
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::extract_expression {
                    ast::scalar::extract_field_kind::hour,
                    value(ast::literal::kind::time, "04:05:06.789"),
            },
            scope,
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_FALSE(r.saw_aggregate());
    EXPECT_EQ(*r, (tscalar::function_call {
            descriptor(func),
            {
                    time("04:05:06.789"),
            },
    }));
}

TEST_F(analyze_scalar_expression_extract_test, minute_time) {
    auto func = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "extract_minute",
            ttype::int4 {},
            {
                    ttype::time_of_day {},
            },
    });
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::extract_expression {
                    ast::scalar::extract_field_kind::minute,
                    value(ast::literal::kind::time, "04:05:06.789"),
            },
            scope,
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_FALSE(r.saw_aggregate());
    EXPECT_EQ(*r, (tscalar::function_call {
            descriptor(func),
            {
                    time("04:05:06.789"),
            },
    }));
}

TEST_F(analyze_scalar_expression_extract_test, second_time) {
    auto func = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "extract_second",
            ttype::int4 {},
            {
                    ttype::time_of_day {},
                    ttype::int4 {},
            },
    });
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::extract_expression {
                    ast::scalar::extract_field_kind::second,
                    value(ast::literal::kind::time, "04:05:06.789"),
            },
            scope,
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_FALSE(r.saw_aggregate());
    EXPECT_EQ(*r, (tscalar::function_call {
            descriptor(func),
            {
                    time("04:05:06.789"),
                    precision(9),
            },
    }));
}

TEST_F(analyze_scalar_expression_extract_test, second_precision_time) {
    auto func = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "extract_second",
            ttype::int4 {},
            {
                    ttype::time_of_day {},
                    ttype::int4 {},
            },
    });
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::extract_expression {
                    ast::scalar::extract_field_kind::second,
                    1,
                    value(ast::literal::kind::time, "04:05:06.789"),
            },
            scope,
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_FALSE(r.saw_aggregate());
    EXPECT_EQ(*r, (tscalar::function_call {
            descriptor(func),
            {
                    time("04:05:06.789"),
                    precision(1),
            },
    }));
}

TEST_F(analyze_scalar_expression_extract_test, year_time) {
    auto func = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "extract_year",
            ttype::int4 {},
            {
                    ttype::time_of_day {},
            },
    });
    invalid(sql_analyzer_code::function_not_found, ast::scalar::extract_expression {
            ast::scalar::extract_field_kind::year,
            value(ast::literal::kind::time, "04:05:06.789"),
    });
}

TEST_F(analyze_scalar_expression_extract_test, prefix_to_month_timestamp) {
    auto func = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "extract_year_to_month",
            ttype::date {},
            {
                    ttype::time_point {},
            },
    });
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::extract_expression {
                    ast::scalar::extract_field_kind::year_to_month,
                    value(ast::literal::kind::timestamp, "2001-02-03 04:05:06.789"),
            },
            scope,
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_FALSE(r.saw_aggregate());
    EXPECT_EQ(*r, (tscalar::function_call {
            descriptor(func),
            {
                    timestamp("2001-02-03 04:05:06.789"),
            },
    }));
}

TEST_F(analyze_scalar_expression_extract_test, prefix_to_day_timestamp) {
    auto func = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "extract_year_to_day",
            ttype::date {},
            {
                    ttype::time_point {},
            },
    });
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::extract_expression {
                    ast::scalar::extract_field_kind::year_to_day,
                    value(ast::literal::kind::timestamp, "2001-02-03 04:05:06.789"),
            },
            scope,
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_FALSE(r.saw_aggregate());
    EXPECT_EQ(*r, (tscalar::function_call {
            descriptor(func),
            {
                    timestamp("2001-02-03 04:05:06.789"),
            },
    }));
}

TEST_F(analyze_scalar_expression_extract_test, prefix_to_hour_timestamp) {
    auto func = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "extract_year_to_hour",
            ttype::time_point {},
            {
                    ttype::time_point {},
            },
    });
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::extract_expression {
                    ast::scalar::extract_field_kind::year_to_hour,
                    value(ast::literal::kind::timestamp, "2001-02-03 04:05:06.789"),
            },
            scope,
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_FALSE(r.saw_aggregate());
    EXPECT_EQ(*r, (tscalar::function_call {
            descriptor(func),
            {
                    timestamp("2001-02-03 04:05:06.789"),
            },
    }));
}

TEST_F(analyze_scalar_expression_extract_test, prefix_to_minute_timestamp) {
    auto func = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "extract_year_to_minute",
            ttype::time_point {},
            {
                    ttype::time_point {},
            },
    });
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::extract_expression {
                    ast::scalar::extract_field_kind::year_to_minute,
                    value(ast::literal::kind::timestamp, "2001-02-03 04:05:06.789"),
            },
            scope,
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_FALSE(r.saw_aggregate());
    EXPECT_EQ(*r, (tscalar::function_call {
            descriptor(func),
            {
                    timestamp("2001-02-03 04:05:06.789"),
            },
    }));
}

TEST_F(analyze_scalar_expression_extract_test, prefix_to_second_timestamp) {
    auto func = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "extract_year_to_second",
            ttype::time_point {},
            {
                    ttype::time_point {},
                    ttype::int4 {},
            },
    });
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::extract_expression {
                    ast::scalar::extract_field_kind::year_to_second,
                    value(ast::literal::kind::timestamp, "2001-02-03 04:05:06.789"),
            },
            scope,
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_FALSE(r.saw_aggregate());
    EXPECT_EQ(*r, (tscalar::function_call {
            descriptor(func),
            {
                    timestamp("2001-02-03 04:05:06.789"),
                    precision(9),
            },
    }));
}

TEST_F(analyze_scalar_expression_extract_test, prefix_to_second_precision_timestamp) {
    auto func = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "extract_year_to_second",
            ttype::time_point {},
            {
                    ttype::time_point {},
                    ttype::int4 {},
            },
    });
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::extract_expression {
                    ast::scalar::extract_field_kind::year_to_second,
                    3,
                    value(ast::literal::kind::timestamp, "2001-02-03 04:05:06.789"),
            },
            scope,
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_FALSE(r.saw_aggregate());
    EXPECT_EQ(*r, (tscalar::function_call {
            descriptor(func),
            {
                    timestamp("2001-02-03 04:05:06.789"),
                    precision(3),
            },
    }));
}

TEST_F(analyze_scalar_expression_extract_test, prefix_date_timestamp) {
    auto func = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "date",
            ttype::date {},
            {
                    ttype::time_point {},
            },
    });
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::extract_expression {
                    ast::scalar::extract_field_kind::date,
                    value(ast::literal::kind::timestamp, "2001-02-03 04:05:06.789"),
            },
            scope,
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_FALSE(r.saw_aggregate());
    EXPECT_EQ(*r, (tscalar::function_call {
            descriptor(func),
            {
                    timestamp("2001-02-03 04:05:06.789"),
            },
    }));
}

TEST_F(analyze_scalar_expression_extract_test, prefix_to_month_timestamptz) {
    auto func = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "extract_year_to_month",
            ttype::date {},
            {
                    ttype::time_point { ttype::with_time_zone },
            },
    });
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::extract_expression {
                    ast::scalar::extract_field_kind::year_to_month,
                    value(ast::literal::kind::timestamp_with_time_zone, "2001-02-03 04:05:06.789"),
            },
            scope,
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_FALSE(r.saw_aggregate());
    EXPECT_EQ(*r, (tscalar::function_call {
            descriptor(func),
            {
                    timestamp("2001-02-03 04:05:06.789", true),
            },
    }));
}

TEST_F(analyze_scalar_expression_extract_test, prefix_to_day_timestamptz) {
    auto func = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "extract_year_to_day",
            ttype::date {},
            {
                    ttype::time_point { ttype::with_time_zone },
            },
    });
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::extract_expression {
                    ast::scalar::extract_field_kind::year_to_day,
                    value(ast::literal::kind::timestamp_with_time_zone, "2001-02-03 04:05:06.789"),
            },
            scope,
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_FALSE(r.saw_aggregate());
    EXPECT_EQ(*r, (tscalar::function_call {
            descriptor(func),
            {
                    timestamp("2001-02-03 04:05:06.789", true),
            },
    }));
}

TEST_F(analyze_scalar_expression_extract_test, prefix_to_hour_timestamptz) {
    auto func = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "extract_year_to_hour",
            ttype::time_point {},
            {
                    ttype::time_point { ttype::with_time_zone },
            },
    });
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::extract_expression {
                    ast::scalar::extract_field_kind::year_to_hour,
                    value(ast::literal::kind::timestamp_with_time_zone, "2001-02-03 04:05:06.789"),
            },
            scope,
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_FALSE(r.saw_aggregate());
    EXPECT_EQ(*r, (tscalar::function_call {
            descriptor(func),
            {
                    timestamp("2001-02-03 04:05:06.789", true),
            },
    }));
}

TEST_F(analyze_scalar_expression_extract_test, prefix_to_minute_timestamptz) {
    auto func = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "extract_year_to_minute",
            ttype::time_point {},
            {
                    ttype::time_point { ttype::with_time_zone },
            },
    });
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::extract_expression {
                    ast::scalar::extract_field_kind::year_to_minute,
                    value(ast::literal::kind::timestamp_with_time_zone, "2001-02-03 04:05:06.789"),
            },
            scope,
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_FALSE(r.saw_aggregate());
    EXPECT_EQ(*r, (tscalar::function_call {
            descriptor(func),
            {
                    timestamp("2001-02-03 04:05:06.789", true),
            },
    }));
}

TEST_F(analyze_scalar_expression_extract_test, prefix_to_second_timestamptz) {
    auto func = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "extract_year_to_second",
            ttype::time_point {},
            {
                    ttype::time_point { ttype::with_time_zone },
                    ttype::int4 {},
            },
    });
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::extract_expression {
                    ast::scalar::extract_field_kind::year_to_second,
                    value(ast::literal::kind::timestamp_with_time_zone, "2001-02-03 04:05:06.789"),
            },
            scope,
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_FALSE(r.saw_aggregate());
    EXPECT_EQ(*r, (tscalar::function_call {
            descriptor(func),
            {
                    timestamp("2001-02-03 04:05:06.789", true),
                    precision(9),
            },
    }));
}

TEST_F(analyze_scalar_expression_extract_test, prefix_to_second_precision_timestamptz) {
    auto func = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "extract_year_to_second",
            ttype::time_point {},
            {
                    ttype::time_point { ttype::with_time_zone },
                    ttype::int4 {},
            },
    });
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::extract_expression {
                    ast::scalar::extract_field_kind::year_to_second,
                    3,
                    value(ast::literal::kind::timestamp_with_time_zone, "2001-02-03 04:05:06.789"),
            },
            scope,
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_FALSE(r.saw_aggregate());
    EXPECT_EQ(*r, (tscalar::function_call {
            descriptor(func),
            {
                    timestamp("2001-02-03 04:05:06.789", true),
                    precision(3),
            },
    }));
}

TEST_F(analyze_scalar_expression_extract_test, prefix_date_timestamptz) {
    auto func = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "date",
            ttype::date {},
            {
                    ttype::time_point { ttype::with_time_zone },
            },
    });
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::extract_expression {
                    ast::scalar::extract_field_kind::date,
                    value(ast::literal::kind::timestamp_with_time_zone, "2001-02-03 04:05:06.789"),
            },
            scope,
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_FALSE(r.saw_aggregate());
    EXPECT_EQ(*r, (tscalar::function_call {
            descriptor(func),
            {
                    timestamp("2001-02-03 04:05:06.789", true),
            },
    }));
}

TEST_F(analyze_scalar_expression_extract_test, prefix_to_month_date) {
    auto func = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "extract_year_to_month",
            ttype::date {},
            {
                    ttype::date {},
            },
    });
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::extract_expression {
                    ast::scalar::extract_field_kind::year_to_month,
                    value(ast::literal::kind::date, "2001-02-03"),
            },
            scope,
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_FALSE(r.saw_aggregate());
    EXPECT_EQ(*r, (tscalar::function_call {
            descriptor(func),
            {
                    date("2001-02-03"),
            },
    }));
}

TEST_F(analyze_scalar_expression_extract_test, prefix_to_day_date) {
    auto func = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "extract_year_to_day",
            ttype::date {},
            {
                    ttype::date {},
            },
    });
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::extract_expression {
                    ast::scalar::extract_field_kind::year_to_day,
                    value(ast::literal::kind::date, "2001-02-03"),
            },
            scope,
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_FALSE(r.saw_aggregate());
    EXPECT_EQ(*r, (tscalar::function_call {
            descriptor(func),
            {
                    date("2001-02-03"),
            },
    }));
}

TEST_F(analyze_scalar_expression_extract_test, prefix_date_date) {
    auto func = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "date",
            ttype::date {},
            {
                    ttype::date {},
            },
    });
    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::extract_expression {
                    ast::scalar::extract_field_kind::date,
                    value(ast::literal::kind::date, "2001-02-03"),
            },
            scope,
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_FALSE(r.saw_aggregate());
    EXPECT_EQ(*r, (tscalar::function_call {
            descriptor(func),
            {
                    date("2001-02-03"),
            },
    }));
}

} // namespace mizugaki::analyzer::details
