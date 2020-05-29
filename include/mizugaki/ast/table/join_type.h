#pragma once

#include <ostream>
#include <string>
#include <string_view>

#include <takatori/util/enum_set.h>

namespace mizugaki::ast::table {

/**
 * @brief represents a kind of join types.
 * @note `7.7 <joined table>` - `<join type>`, etc.
 */
enum class join_type {
    /// @brief `CROSS`
    cross,
    /// @brief `INNER`
    inner,
    /// @brief `LEFT OUTER`
    left_outer,
    /// @brief `RIGHT OUTER`
    right_outer,
    /// @brief `FULL OUTER`
    full_outer,
    /// @brief `NATURAL INNER`
    natural_inner,
    /// @brief `NATURAL LEFT OUTER`
    natural_left_outer,
    /// @brief `NATURAL RIGHT OUTER`
    natural_right_outer,
    /// @brief `NATURAL FULL OUTER`
    natural_full_outer,
    /// @brief `UNION`
    union_,
};

/// @brief set of literal join_kind.
using join_kind_set = ::takatori::util::enum_set<
        join_type,
        join_type::cross,
        join_type::union_>;

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(join_type value) noexcept {
    using namespace std::string_view_literals;
    using kind = join_type;
    switch (value) {
        case kind::cross: return "cross"sv;
        case kind::inner: return "inner"sv;
        case kind::left_outer: return "left_outer"sv;
        case kind::right_outer: return "right_outer"sv;
        case kind::full_outer: return "full_outer"sv;
        case kind::natural_inner: return "natural_inner"sv;
        case kind::natural_left_outer: return "natural_left_outer"sv;
        case kind::natural_right_outer: return "natural_right_outer"sv;
        case kind::natural_full_outer: return "natural_full_outer"sv;
        case kind::union_: return "union"sv;
    }
    std::abort();
}

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
inline std::ostream& operator<<(std::ostream& out, join_type value) {
    return out << to_string_view(value);
}

} // namespace mizugaki::ast::table
