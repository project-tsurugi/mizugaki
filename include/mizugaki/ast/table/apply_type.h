#pragma once

#include <ostream>
#include <string>
#include <string_view>

#include <takatori/util/enum_set.h>

namespace mizugaki::ast::table {

/**
 * @brief represents a kind of table-valued function invocation types.
 * @note This is a non-standard extension.
 */
enum class apply_type {
    /// @brief `CROSS`
    cross,
    /// @brief `OUTER`
    outer,
};

/// @brief set of literal apply_kind.
using apply_kind_set = ::takatori::util::enum_set<
        apply_type,
        apply_type::cross,
        apply_type::outer>;

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
constexpr std::string_view to_string_view(apply_type value) noexcept {
    using namespace std::string_view_literals;
    using kind = apply_type;
    switch (value) {
        case kind::cross: return "cross"sv;
        case kind::outer: return "outer"sv;
    }
    std::abort();
}

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
inline std::ostream& operator<<(std::ostream& out, apply_type value) {
    return out << to_string_view(value);
}

} // namespace mizugaki::ast::table
