#pragma once

#include <ostream>
#include <string>
#include <string_view>

#include <takatori/util/enum_set.h>
#include <takatori/util/meta_type.h>

namespace mizugaki::ast::scalar {

/**
 * @brief represents a kind of built-in function.
 * @note `6.17 <numeric value function>`
 * @note `6.18 <string value function>`
 * @note `6.19 <datetime value function>`
 * @note `6.20 <interval value function>`
 * @note `6.21 <case expression>`
 */
enum class builtin_function_kind {
    abs,
    // FIXME: more

    /**
     * @brief `NULLIF`
     * @note `6.21 <case expression>` - `<case abbreviation>`
     */
    nullif,

    /**
     * @brief `COALESCE`
     * @note `6.21 <case expression>` - `<case abbreviation>`
     */
    coalesce,
};

/// @brief set of builtin_function_kind kind.
using builtin_function_kind_set = ::takatori::util::enum_set<
        builtin_function_kind,
        builtin_function_kind::abs,
        builtin_function_kind::abs>; // FIXME: adjust

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(builtin_function_kind value) noexcept {
    using namespace std::string_view_literals;
    using kind = builtin_function_kind;
    switch (value) {
        case kind::abs: return "abs"sv;
        case kind::nullif: return "nullif"sv;
        case kind::coalesce: return "coalesce"sv;
    }
    std::abort();
}

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
inline std::ostream& operator<<(std::ostream& out, builtin_function_kind value) {
    return out << to_string_view(value);
}

} // namespace mizugaki::ast::scalar
