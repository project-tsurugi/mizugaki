#pragma once

#include <ostream>
#include <string>
#include <string_view>

#include <takatori/util/enum_set.h>

namespace mizugaki::ast::literal {

/**
 * @brief represents a kind of boolean value.
 */
enum class boolean_kind {
    /// @brief `TRUE`.
    true_,
    /// @brief `FALSE`.
    false_,
    /// @brief `UNKNOWN`.
    unknown,
};

/// @brief set of boolean kind.
using boolean_kind_set = ::takatori::util::enum_set<
        boolean_kind,
        boolean_kind::true_,
        boolean_kind::unknown>;

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(boolean_kind value) noexcept {
    using namespace std::string_view_literals;
    switch (value) {
        case boolean_kind::true_: return "TRUE"sv;
        case boolean_kind::false_: return "FALSE"sv;
        case boolean_kind::unknown: return "UNKNOWN"sv;
    }
    std::abort();
}

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
inline std::ostream& operator<<(std::ostream& out, boolean_kind value) {
    return out << to_string_view(value);
}

} // namespace mizugaki::ast::literal
