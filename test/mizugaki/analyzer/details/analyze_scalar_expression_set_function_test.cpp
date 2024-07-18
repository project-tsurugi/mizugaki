#include <mizugaki/analyzer/details/analyze_scalar_expression.h>

#include <gtest/gtest.h>

#include <takatori/value/primitive.h>
#include <takatori/type/primitive.h>
#include <takatori/type/decimal.h>
#include <takatori/type/time_point.h>

#include <takatori/scalar/cast.h>

#include <yugawara/binding/factory.h>

#include <yugawara/aggregate/declaration.h>

#include <yugawara/extension/scalar/aggregate_function_call.h>

#include <mizugaki/ast/type/simple.h>
#include <mizugaki/ast/type/character_string.h>
#include <mizugaki/ast/type/decimal.h>

#include <mizugaki/ast/scalar/binary_expression.h>
#include <mizugaki/ast/scalar/builtin_set_function_invocation.h>
#include <mizugaki/ast/scalar/cast_expression.h>

#include "test_parent.h"

namespace mizugaki::analyzer::details {

using namespace ::mizugaki::analyzer::testing;

class analyze_scalar_expression_set_function_test : public test_parent {
protected:
    ::takatori::descriptor::aggregate_function descriptor(std::shared_ptr<::yugawara::aggregate::declaration const> decl) {
        return ::yugawara::binding::factory {}(std::move(decl));
    }

    void invalid(ast::scalar::expression const& expression) {
        auto r = analyze_scalar_expression(
                context(),
                expression,
                {},
                {});
        EXPECT_FALSE(r) << diagnostics();
        EXPECT_NE(count_error(), 0);
    }

    void invalid(sql_analyzer_code code, ast::scalar::expression const& expression) {
        invalid(expression);
        EXPECT_TRUE(find_error(code));
    }

    ast::scalar::literal_expression erroneous_expression() {
        return literal(string("INVALID"));
    }

    ast::type::character_string erroneous_type() {
        return ast::type::character_string { ast::type::kind::character_varying, 0 };
    }
};

TEST_F(analyze_scalar_expression_set_function_test, builtin_simple) {
    auto func = set_functions_->add(::yugawara::aggregate::declaration {
            ::yugawara::aggregate::declaration::minimum_builtin_function_id + 1,
            "count",
            ttype::int8 {},
            {},
            true,
    });

    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::builtin_set_function_invocation {
                    ast::scalar::builtin_set_function_kind::count,
                    {},
                    {},
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_TRUE(r.saw_aggregate());
    EXPECT_EQ(*r, (::yugawara::extension::scalar::aggregate_function_call {
            descriptor(func),
            {},
    }));
}

TEST_F(analyze_scalar_expression_set_function_test, builtin_args) {
    auto empty = set_functions_->add(::yugawara::aggregate::declaration {
            ::yugawara::aggregate::declaration::minimum_builtin_function_id + 1,
            "count",
            ttype::int8 {},
            {},
            true,
    });
    auto args = set_functions_->add(::yugawara::aggregate::declaration {
            ::yugawara::aggregate::declaration::minimum_builtin_function_id + 2,
            "count",
            ttype::int8 {},
            {
                    ttype::int8 {},
            },
            true,
    });

    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::builtin_set_function_invocation {
                    ast::scalar::builtin_set_function_kind::count,
                    ast::scalar::set_quantifier::all,
                    {
                            literal(number("1")),
                    },
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_TRUE(r.saw_aggregate());
    EXPECT_EQ(*r, (::yugawara::extension::scalar::aggregate_function_call {
            descriptor(args),
            {
                    immediate(1),
            },
    }));
}

TEST_F(analyze_scalar_expression_set_function_test, builtin_distinct) {
    auto all = set_functions_->add(::yugawara::aggregate::declaration {
            ::yugawara::aggregate::declaration::minimum_builtin_function_id + 1,
            "sum",
            ttype::int8 {},
            {
                    ttype::int8 {},
            },
            true,
    });
    auto distinct = set_functions_->add(::yugawara::aggregate::declaration {
            ::yugawara::aggregate::declaration::minimum_builtin_function_id + 2,
            std::string { "sum" }.append(::yugawara::aggregate::declaration::name_suffix_distinct),
            ttype::int8 {},
            {
                    ttype::int8 {},
            },
            true,
    });

    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::builtin_set_function_invocation {
                    ast::scalar::builtin_set_function_kind::sum,
                    ast::scalar::set_quantifier::distinct,
                    {
                            literal(number("1")),
                    },
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_TRUE(r.saw_aggregate());
    EXPECT_EQ(*r, (::yugawara::extension::scalar::aggregate_function_call {
            descriptor(distinct),
            {
                    immediate(1),
            },
    }));
}

TEST_F(analyze_scalar_expression_set_function_test, builtin_overload) {
    auto f_int4 = set_functions_->add(::yugawara::aggregate::declaration {
            ::yugawara::aggregate::declaration::minimum_builtin_function_id + 1,
            "max",
            ttype::int8 {},
            {
                    ttype::int4 {},
            },
            true,
    });
    auto f_int8 = set_functions_->add(::yugawara::aggregate::declaration {
            ::yugawara::aggregate::declaration::minimum_builtin_function_id + 2,
            "max",
            ttype::int8 {},
            {
                    ttype::int8 {},
            },
            true,
    });
    auto f_decimal = set_functions_->add(::yugawara::aggregate::declaration {
            ::yugawara::aggregate::declaration::minimum_builtin_function_id + 3,
            "max",
            ttype::decimal {},
            {
                    ttype::decimal {},
            },
            true,
    });

    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::builtin_set_function_invocation {
                    ast::scalar::builtin_set_function_kind::max,
                    ast::scalar::set_quantifier::all,
                    {
                            literal(number("1")),
                    },
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_TRUE(r.saw_aggregate());
    EXPECT_EQ(*r, (::yugawara::extension::scalar::aggregate_function_call {
            descriptor(f_int8),
            {
                    immediate(1),
            },
    }));
}

TEST_F(analyze_scalar_expression_set_function_test, builtin_overload_limited_decimal) {
    auto f_int4 = set_functions_->add(::yugawara::aggregate::declaration {
            ::yugawara::aggregate::declaration::minimum_builtin_function_id + 1,
            "max",
            ttype::int8 {},
            {
                    ttype::int4 {},
            },
            true,
    });
    auto f_int8 = set_functions_->add(::yugawara::aggregate::declaration {
            ::yugawara::aggregate::declaration::minimum_builtin_function_id + 2,
            "max",
            ttype::int8 {},
            {
                    ttype::int8 {},
            },
            true,
    });
    auto f_decimal = set_functions_->add(::yugawara::aggregate::declaration {
            ::yugawara::aggregate::declaration::minimum_builtin_function_id + 3,
            "max",
            ttype::decimal {},
            {
                    ttype::decimal {},
            },
            true,
    });

    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::builtin_set_function_invocation {
                    ast::scalar::builtin_set_function_kind::max,
                    ast::scalar::set_quantifier::all,
                    {
                            ast::scalar::cast_expression {
                                    ast::scalar::cast_operator::cast,
                                    literal(number("1")),
                                    ast::type::decimal {
                                            ast::type::kind::decimal,
                                            5,
                                            0,
                                    },
                            },
                    },
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_TRUE(r.saw_aggregate());
    EXPECT_EQ(*r, (::yugawara::extension::scalar::aggregate_function_call {
            descriptor(f_decimal),
            {
                    tscalar::cast {
                            ttype::decimal { 5, 0 },
                            tscalar::cast::loss_policy_type::ignore,
                            immediate(1),
                    },
            },
    }));
}

TEST_F(analyze_scalar_expression_set_function_test, builtin_overload_timestamp) {
    auto f_ts = set_functions_->add(::yugawara::aggregate::declaration {
            ::yugawara::aggregate::declaration::minimum_builtin_function_id + 1,
            "max",
            ttype::time_point {},
            {
                    ttype::time_point {},
            },
            true,
    });
    auto f_ts_tz = set_functions_->add(::yugawara::aggregate::declaration {
            ::yugawara::aggregate::declaration::minimum_builtin_function_id + 2,
            "max",
            ttype::time_point { ttype::with_time_zone },
            {
                    ttype::time_point { ttype::with_time_zone },
            },
            true,
    });

    auto&& ctxt = context();
    auto v = vd("v", ttype::time_point {});
    query_scope scope {};
    auto&& rinfo = scope.add({});
    rinfo.add({ {}, v, "v" });

    auto r = analyze_scalar_expression(
            ctxt,
            ast::scalar::builtin_set_function_invocation {
                    ast::scalar::builtin_set_function_kind::max,
                    ast::scalar::set_quantifier::all,
                    {
                            vref(id("v")),
                    },
            },
            scope,
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_TRUE(r.saw_aggregate());
    EXPECT_EQ(*r, (::yugawara::extension::scalar::aggregate_function_call {
            descriptor(f_ts),
            {
                    vref(v),
            },
    }));
}

TEST_F(analyze_scalar_expression_set_function_test, builtin_overload_timestamp_tz) {
    auto f_ts = set_functions_->add(::yugawara::aggregate::declaration {
            ::yugawara::aggregate::declaration::minimum_builtin_function_id + 1,
            "max",
            ttype::time_point {},
            {
                    ttype::time_point {},
            },
            true,
    });
    auto f_ts_tz = set_functions_->add(::yugawara::aggregate::declaration {
            ::yugawara::aggregate::declaration::minimum_builtin_function_id + 2,
            "max",
            ttype::time_point { ttype::with_time_zone },
            {
                    ttype::time_point { ttype::with_time_zone },
            },
            true,
    });

    auto&& ctxt = context();
    auto v = vd("v", ttype::time_point { ttype::with_time_zone });
    query_scope scope {};
    auto&& rinfo = scope.add({});
    rinfo.add({ {}, v, "v" });

    auto r = analyze_scalar_expression(
            ctxt,
            ast::scalar::builtin_set_function_invocation {
                    ast::scalar::builtin_set_function_kind::max,
                    ast::scalar::set_quantifier::all,
                    {
                            vref(id("v")),
                    },
            },
            scope,
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_TRUE(r.saw_aggregate());
    EXPECT_EQ(*r, (::yugawara::extension::scalar::aggregate_function_call {
            descriptor(f_ts_tz),
            {
                    vref(v),
            },
    }));
}

TEST_F(analyze_scalar_expression_set_function_test, builtin_invalid_argument) {
    auto func = set_functions_->add(::yugawara::aggregate::declaration {
            ::yugawara::aggregate::declaration::minimum_builtin_function_id + 2,
            "count",
            ttype::int8 {},
            {
                    ttype::character { ttype::varying },
            },
            true,
    });
    invalid(ast::scalar::builtin_set_function_invocation {
            ast::scalar::builtin_set_function_kind::count,
            {},
            {
                    erroneous_expression(),
            },
    });
}

TEST_F(analyze_scalar_expression_set_function_test, builtin_missing) {
    invalid(diagnostic_code::function_not_found, ast::scalar::builtin_set_function_invocation {
            ast::scalar::builtin_set_function_kind::count,
            {},
            {},
    });
}

TEST_F(analyze_scalar_expression_set_function_test, builtin_ambiguous) {
    set_functions_->add(::yugawara::aggregate::declaration {
            ::yugawara::aggregate::declaration::minimum_builtin_function_id + 1,
            "count",
            ttype::int8 {},
            {},
            true,
    });
    set_functions_->add(::yugawara::aggregate::declaration {
            ::yugawara::aggregate::declaration::minimum_builtin_function_id + 2,
            "count",
            ttype::int8 {},
            {},
            true,
    });
    invalid(diagnostic_code::function_ambiguous, ast::scalar::builtin_set_function_invocation {
            ast::scalar::builtin_set_function_kind::count,
            {},
            {},
    });
}

} // namespace mizugaki::analyzer::details
