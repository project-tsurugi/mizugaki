#pragma once

#include <takatori/descriptor/variable.h>

#include <takatori/type/primitive.h>
#include <takatori/type/character.h>

#include <takatori/value/primitive.h>
#include <takatori/value/character.h>

#include <takatori/scalar/immediate.h>
#include <takatori/scalar/variable_reference.h>

#include <takatori/relation/graph.h>

#include <takatori/util/downcast.h>
#include <takatori/util/maybe_shared_ptr.h>
#include <takatori/util/string_builder.h>

#include <yugawara/binding/factory.h>

#include <mizugaki/ast/name/simple.h>
#include <mizugaki/ast/name/qualified.h>

#include <mizugaki/ast/literal/numeric.h>
#include <mizugaki/ast/literal/string.h>

#include <mizugaki/ast/scalar/literal_expression.h>
#include <mizugaki/ast/scalar/variable_reference.h>

#include <mizugaki/analyzer/details/analyzer_context.h>

namespace mizugaki::analyzer::testing {

namespace tstatement = ::takatori::statement;
namespace trelation = ::takatori::relation;
namespace tscalar = ::takatori::scalar;
namespace tvalue = ::takatori::value;
namespace ttype = ::takatori::type;

using diagnostic_code = details::analyzer_context::diagnostic_type::code_type;

using ::takatori::util::downcast;

inline std::string diagnostics_to_string(details::analyzer_context const& context) {
    ::takatori::util::string_builder builder;
    for (auto&& d : context.diagnostics()) {
        builder << d << "\n";
    }
    return builder << ::takatori::util::string_builder::to_string;
}

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

inline tscalar::immediate immediate(tvalue::int8::entity_type value) {
    return tscalar::immediate {
            tvalue::int8 { value },
            ttype::int8 {},
    };
}

inline tscalar::immediate immediate(std::string_view value) {
    return tscalar::immediate {
            tvalue::character { value },
            ttype::character { ttype::varying, {} },
    };
}

inline tscalar::immediate immediate_bool(bool value = true) {
    return tscalar::immediate {
            tvalue::boolean { value },
            ttype::boolean {},
    };
}

inline ast::name::simple id(std::string_view token = "id", bool regular = true) {
    return ast::name::simple {
            ast::common::chars { token },
            regular ? ast::name::identifier_kind::regular : ast::name::identifier_kind::delimited,
    };
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

template<class T>
inline ::takatori::util::optional_ptr<T const> find_first(trelation::graph_type const& graph) {
    for (auto&& node : graph) {
        if (!node.input_ports().empty()) {
            continue;
        }
        if (auto ptr = downcast<T>(&node)) {
            return ::takatori::util::optional_ptr { ptr };
        }
    }
    return {};
}

template<class T>
inline ::takatori::util::optional_ptr<T const> find_last(trelation::graph_type const& graph) {
    for (auto&& node : graph) {
        if (!node.output_ports().empty()) {
            continue;
        }
            if (auto ptr = downcast<T>(&node)) {
            return ::takatori::util::optional_ptr { ptr };
        }
    }
    return {};
}

template<class T = trelation::expression, class U = trelation::expression>
inline ::takatori::util::optional_ptr<T const> find_next(U const& node) {
    if (auto opposite = node.output().opposite()) {
        if (auto ptr = downcast<T>(&opposite->owner())) {
            return ::takatori::util::optional_ptr<T const> { ptr };
        }
    }
    return {};
}

template<class T = trelation::expression, class U = trelation::expression>
inline ::takatori::util::optional_ptr<T const> find_prev(U const& node) {
    if (auto opposite = node.input().opposite()) {
        if (auto ptr = downcast<T>(&opposite->owner())) {
            return ::takatori::util::optional_ptr<T const> { ptr };
        }
    }
    return {};
}

} // namespace mizugaki::analyzer::testing
