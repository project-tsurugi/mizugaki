#pragma once

#include <type_traits>

#include <mizugaki/ast/name/simple.h>
#include <mizugaki/ast/scalar/variable_reference.h>

namespace mizugaki::ast::testing {

template<class T>
inline T& as_lvref(T&& t) {
    return t;
}

inline name::simple id(std::string_view token = "id") {
    return name::simple { token };
}

inline scalar::variable_reference vref(name::name&& n = id()) {
    return scalar::variable_reference { std::move(n) };
}

} // namespace mizugaki::ast::testing
