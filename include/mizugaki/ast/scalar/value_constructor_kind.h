#pragma once

#include <ostream>
#include <string>
#include <string_view>

#include <takatori/util/enum_set.h>

namespace mizugaki::ast::scalar {

/**
 * @brief represents a kind of value constructor.
 * @note `6.32 <array value constructor>`
 * @note `7.1 <row value constructor>`
 * @see value_constructor
 */
enum class value_constructor_kind {
    /**
     * @brief `ARRAY`
     */
    array,

    /**
     * @brief `ROW`
     */
    row,
};

/// @brief set of value_constructor_kind kind.
using value_constructor_kind_set = ::takatori::util::enum_set<
        value_constructor_kind,
        value_constructor_kind::array,
        value_constructor_kind::row>;

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(value_constructor_kind value) noexcept {
    using namespace std::string_view_literals;
    using kind = value_constructor_kind;
    switch (value) {
        case kind::array: return "array"sv;
        case kind::row: return "row"sv;
    }
    std::abort();
}

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
inline std::ostream& operator<<(std::ostream& out, value_constructor_kind value) {
    return out << to_string_view(value);
}

} // namespace mizugaki::ast::scalar
