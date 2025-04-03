#include <mizugaki/analyzer/details/analyze_scalar_expression.h>

#include <gtest/gtest.h>

#include <takatori/type/character.h>
#include <takatori/type/octet.h>
#include <takatori/type/date.h>

#include <takatori/scalar/function_call.h>

#include <yugawara/binding/factory.h>

#include <yugawara/function/declaration.h>

#include <mizugaki/ast/type/simple.h>
#include <mizugaki/ast/type/character_string.h>

#include <mizugaki/ast/scalar/binary_expression.h>
#include <mizugaki/ast/scalar/builtin_function_invocation.h>
#include <mizugaki/ast/scalar/function_invocation.h>

#include "test_parent.h"

namespace mizugaki::analyzer::details {

using namespace ::mizugaki::analyzer::testing;

class analyze_scalar_expression_function_test : public test_parent {
protected:
    ::takatori::descriptor::function descriptor(std::shared_ptr<::yugawara::function::declaration const> decl) {
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

TEST_F(analyze_scalar_expression_function_test, builtin_simple) {
    auto func = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "current_date",
            ttype::date {},
            {},
    });

    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::builtin_function_invocation {
                    ast::scalar::builtin_function_kind::current_date,
                    {},
                    {},
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_FALSE(r.saw_aggregate());
    EXPECT_EQ(*r, (tscalar::function_call {
            descriptor(func),
            {},
    }));
}

TEST_F(analyze_scalar_expression_function_test, builtin_args) {
    auto f_char = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "length",
            ttype::int8 {},
            {
                    ttype::character { ttype::varying, {} },
            },
    });

    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::builtin_function_invocation {
                    ast::scalar::builtin_function_kind::length,
                    {
                            literal(string("'testing'")),
                    },
                    {},
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_FALSE(r.saw_aggregate());
    EXPECT_EQ(*r, (tscalar::function_call {
            descriptor(f_char),
            {
                    immediate("testing")
            },
    }));
}

TEST_F(analyze_scalar_expression_function_test, builtin_overload) {
    auto f_char = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "length",
            ttype::int8 {},
            {
                    ttype::character { ttype::varying, {} },
            },
    });
    auto f_binary = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_builtin_function_id + 2,
            "length",
            ttype::int8 {},
            {
                    ttype::octet { ttype::varying, {} },
            },
    });

    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::builtin_function_invocation {
                    ast::scalar::builtin_function_kind::length,
                    {
                            literal(string("'testing'")),
                    },
                    {},
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_FALSE(r.saw_aggregate());
    EXPECT_EQ(*r, (tscalar::function_call {
            descriptor(f_char),
            {
                    immediate("testing")
            },
    }));
}

TEST_F(analyze_scalar_expression_function_test, builtin_mismatch_argument) {
    auto f_binary = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_builtin_function_id + 2,
            "length",
            ttype::int8 {},
            {
                    ttype::octet { ttype::varying, {} },
            },
    });

    invalid(sql_analyzer_code::function_not_found, ast::scalar::builtin_function_invocation {
            ast::scalar::builtin_function_kind::length,
            {
                    literal(string("'testing'")),
            },
            {},
    });
}

TEST_F(analyze_scalar_expression_function_test, builtin_invalid_argument) {
    auto f_char = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "length",
            ttype::int8 {},
            {
                    ttype::character { ttype::varying, {} },
            },
    });

    invalid(ast::scalar::builtin_function_invocation {
            ast::scalar::builtin_function_kind::length,
            {
                    erroneous_expression(),
            },
            {},
    });
}

TEST_F(analyze_scalar_expression_function_test, builtin_missing) {
    invalid(sql_analyzer_code::function_not_found, ast::scalar::builtin_function_invocation {
            ast::scalar::builtin_function_kind::length,
            {
                    literal(string("'testing'")),
            },
            {},
    });
}

TEST_F(analyze_scalar_expression_function_test, builtin_ambiguous) {
    auto f_char1 = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_builtin_function_id + 1,
            "length",
            ttype::int8 {},
            {
                    ttype::character { ttype::varying, {} },
            },
    });
    auto f_char2 = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_builtin_function_id + 2,
            "length",
            ttype::int8 {},
            {
                    ttype::character { ttype::varying, {} },
            },
    });
    invalid(sql_analyzer_code::function_ambiguous, ast::scalar::builtin_function_invocation {
            ast::scalar::builtin_function_kind::length,
            {
                    literal(string("'testing'")),
            },
            {},
    });
}

TEST_F(analyze_scalar_expression_function_test, function_simple) {
    auto func = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_user_function_id + 1,
            "constant",
            ttype::int8 {},
            {},
    });

    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::function_invocation {
                    id("constant"),
                    {},
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_FALSE(r.saw_aggregate());
    EXPECT_EQ(*r, (tscalar::function_call {
            descriptor(func),
            {},
    }));
}

TEST_F(analyze_scalar_expression_function_test, function_arguments) {
    auto func = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_user_function_id + 1,
            "substr",
            ttype::character { ttype::varying, {} },
            {
                    ttype::character { ttype::varying, {} },
                    ttype::int8 {},
            },
    });

    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::function_invocation {
                    id("SUBSTR"),
                    {
                            literal(string("'Hello, world!'")),
                            literal(number("8")),
                    },
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_FALSE(r.saw_aggregate());
    EXPECT_EQ(*r, (tscalar::function_call {
            descriptor(func),
            {
                    immediate("Hello, world!"),
                    immediate(8),
            },
    }));
}

TEST_F(analyze_scalar_expression_function_test, function_overload) {
    auto f_char = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_user_function_id + 1,
            "substr",
            ttype::character { ttype::varying, {} },
            {
                    ttype::character { ttype::varying, {} },
                    ttype::int8 {},
            },
    });
    auto f_octet = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_user_function_id + 2,
            "substr",
            ttype::octet { ttype::varying, {} },
            {
                    ttype::octet { ttype::varying, {} },
                    ttype::int8 {},
            },
    });

    auto r = analyze_scalar_expression(
            context(),
            ast::scalar::function_invocation {
                    id("SUBSTR"),
                    {
                            literal(binary("'CAFEBABE'")),
                            literal(number("3")),
                    },
            },
            {},
            {});
    ASSERT_TRUE(r) << diagnostics();
    expect_no_error();

    EXPECT_FALSE(r.saw_aggregate());
    EXPECT_EQ(*r, (tscalar::function_call {
            descriptor(f_octet),
            {
                    immediate_octet("\xca\xfe\xba\xbe"),
                    immediate(3),
            },
    }));
}

TEST_F(analyze_scalar_expression_function_test, function_mismatch_argument) {
    auto f_char = functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_user_function_id + 1,
            "substr",
            ttype::character { ttype::varying, {} },
            {
                    ttype::character { ttype::varying, {} },
                    ttype::int8 {},
            },
    });

    invalid(sql_analyzer_code::function_not_found, ast::scalar::function_invocation {
            id("SUBSTR"),
            {
                    literal(binary("'CAFEBABE'")),
                    literal(number("3")),
            },
    });
}

TEST_F(analyze_scalar_expression_function_test, function_invalid_argument) {
    invalid(ast::scalar::function_invocation {
            id("invalid"),
            {
                    erroneous_expression(),
            },
    });
}

TEST_F(analyze_scalar_expression_function_test, function_ambiguous) {
    functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_user_function_id + 1,
            "substr",
            ttype::character { ttype::varying, {} },
            {
                    ttype::character { ttype::varying, {} },
                    ttype::int8 {},
            },
    });
    functions_->add(::yugawara::function::declaration {
            ::yugawara::function::declaration::minimum_user_function_id + 2,
            "substr",
            ttype::character { ttype::varying, {} },
            {
                    ttype::character { ttype::varying, {} },
                    ttype::int8 {},
            },
    });

    invalid(sql_analyzer_code::function_ambiguous, ast::scalar::function_invocation {
            id("SUBSTR"),
            {
                    literal(string("'Hello, world!'")),
                    literal(number("8")),
            },
    });
}

} // namespace mizugaki::analyzer::details
