#pragma once

#include <ostream>
#include <string>
#include <string_view>

namespace mizugaki::ast::statement {

/**
 * @brief represents an option of view_definition.
 * @note `11.21 <view definition>`
 */
enum class view_definition_option {
    /// @brief redefine for existing view.
    or_replace,
    /// @brief view lives temporary.
    temporary,
    /// @brief defines a recursive view.
    recursive,
    /// @brief ignores definition for existing views (exclusive with `or_replace`).
    if_not_exists,

    // FIXME with [local|cascaded] check option
};

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(view_definition_option value) noexcept {
    using namespace std::string_view_literals;
    using kind = view_definition_option;
    switch (value) {
        case kind::or_replace: return "or_replace"sv;
        case kind::temporary: return "temporary"sv;
        case kind::recursive: return "recursive"sv;
        case kind::if_not_exists: return "if_not_exists"sv;
    }
    std::abort();
}

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
inline std::ostream& operator<<(std::ostream& out, view_definition_option value) {
    return out << to_string_view(value);
}

} // namespace mizugaki::ast::statement
