#pragma once

#include <takatori/descriptor/variable.h>

#include <takatori/type/primitive.h>
#include <takatori/type/character.h>

#include <takatori/value/primitive.h>
#include <takatori/value/character.h>

#include <takatori/scalar/immediate.h>
#include <takatori/scalar/variable_reference.h>

#include <takatori/util/maybe_shared_ptr.h>

#include <yugawara/binding/factory.h>

#include <mizugaki/ast/name/simple.h>
#include <mizugaki/ast/name/qualified.h>

#include <mizugaki/ast/literal/numeric.h>
#include <mizugaki/ast/literal/string.h>

#include <mizugaki/ast/scalar/literal_expression.h>
#include <mizugaki/ast/scalar/variable_reference.h>

#include <mizugaki/analyzer/details/analyzer_context.h>

namespace mizugaki::analyzer::testing {

namespace tscalar = ::takatori::scalar;
namespace tvalue = ::takatori::value;
namespace ttype = ::takatori::type;

using diagnostic_code = details::analyzer_context::diagnostic_type::code_type;

inline bool contains(details::analyzer_context const& context, diagnostic_code code) {
    for (auto&& d : context.diagnostics()) {
        if (d.code() == code) {
            return true;
        }
    }
    return false;
}

inline ast::literal::numeric number(std::string_view token = "1") {
    return ast::literal::numeric {
            ast::literal::kind::exact_numeric,
            token,
    };
}

inline ast::literal::string string(std::string_view token = "'x'") {
    return ast::literal::string {
            ast::literal::kind::character_string,
            token,
    };
}

inline ast::scalar::literal_expression literal(ast::literal::literal&& literal = number()) {
    return ast::scalar::literal_expression {
            std::move(literal),
    };
}

inline tscalar::immediate immediate(int value) {
    return tscalar::immediate {
            tvalue::int4 { value },
            ttype::int4 {},
    };
}

inline tscalar::immediate immediate(std::string_view value) {
    return tscalar::immediate {
            tvalue::character { value },
            ttype::character { ttype::varying, value.size() },
    };
}

inline ast::name::simple id(std::string_view token = "id") {
    return ast::name::simple { token };
}

template<class... Args>
inline std::enable_if_t<sizeof...(Args) >= 2, ast::name::qualified> id(Args&&... args) {
    return ast::name::qualified {
            ast::name::simple(std::forward<Args>(args))...,
    };
}

inline ast::scalar::variable_reference vref(ast::name::name&& n = id()) {
    return ast::scalar::variable_reference { std::move(n) };
}


inline ::takatori::descriptor::variable vdesc(std::string_view name = "id") {
    return ::yugawara::binding::factory {}.stream_variable(name);
}

inline ::takatori::scalar::variable_reference vref(::takatori::descriptor::variable v) {
    return ::takatori::scalar::variable_reference { std::move(v) };
}

template<class T>
inline std::enable_if_t<!std::is_reference_v<T>, std::shared_ptr<std::remove_const_t<T>>> share(T&& element) {
    return std::make_shared<std::remove_const_t<T>>(std::forward<T>(element));
}

template<class T>
inline std::enable_if_t<std::is_reference_v<T>, ::takatori::util::maybe_shared_ptr<std::remove_reference_t<T>>> share(T&& element) {
    return ::takatori::util::maybe_shared_ptr<std::remove_reference_t<T>> { std::addressof(element) };
}

} // namespace mizugaki::analyzer::testing
