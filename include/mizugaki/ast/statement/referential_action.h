#pragma once

#include <ostream>
#include <string>
#include <string_view>

namespace mizugaki::ast::statement {

/**
 * @brief represents an action on referring target was changed.
 * @see `11.8 <referential constraint definition>` - `<referential action>`
 */
enum class referential_action {
    /// @brief follows the change (delete/update).
    cascade,
    /// @brief sets `NULL` to the source column(s).
    set_null,
    /// @brief sets default value to the individual source column(s).
    set_default,
    /// @brief restricts the action itself.
    restrict,
    /// @brief does nothing.
    no_action,
};

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(referential_action value) noexcept {
    using namespace std::string_view_literals;
    using kind = referential_action;
    switch (value) {
        case kind::cascade: return "cascade"sv;
        case kind::set_null: return "set_null"sv;
        case kind::set_default: return "set_default"sv;
        case kind::restrict: return "restrict"sv;
        case kind::no_action: return "no_action"sv;
    }
    std::abort();
}

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
inline std::ostream& operator<<(std::ostream& out, referential_action value) {
    return out << to_string_view(value);
}

} // namespace mizugaki::ast::statement
