#pragma once

#include <ostream>
#include <string>
#include <string_view>

#include <takatori/util/enum_set.h>
#include <takatori/util/meta_type.h>

namespace mizugaki::ast::scalar {

/**
 * @brief represents a kind of built-in set function.
 * @note `6.16 <set function specification>`
 */
enum class builtin_set_function_kind {
    /// @brief `AVG`
    avg,
    /// @brief `MAX`
    max,
    /// @brief `MIN`
    min,
    /// @brief `SUM`
    sum,
    /// @brief `EVERY`
    every,
    /// @brief `ANY`
    any,
    /// @brief `SOME`
    some,
    /// @brief `COUNT`
    count,
};

/// @brief set of builtin_set_function_kind kind.
using builtin_set_function_kind_set = ::takatori::util::enum_set<
        builtin_set_function_kind,
        builtin_set_function_kind::avg,
        builtin_set_function_kind::count>;

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(builtin_set_function_kind value) noexcept {
    using namespace std::string_view_literals;
    using kind = builtin_set_function_kind;
    switch (value) {
        case kind::avg: return "avg"sv;
        case kind::max: return "max"sv;
        case kind::min: return "min"sv;
        case kind::sum: return "sum"sv;
        case kind::every: return "every"sv;
        case kind::any: return "any"sv;
        case kind::some: return "some"sv;
        case kind::count: return "count"sv;
    }
    std::abort();
}

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
inline std::ostream& operator<<(std::ostream& out, builtin_set_function_kind value) {
    return out << to_string_view(value);
}

} // namespace mizugaki::ast::scalar
