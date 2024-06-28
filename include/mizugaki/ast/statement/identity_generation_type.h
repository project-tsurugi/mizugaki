#pragma once

#include <ostream>
#include <string>
#include <string_view>

namespace mizugaki::ast::statement {

/**
 * @brief generation type for identity columns.
 * @note `[SQL-2003] 11.4 <column definition>` - `<identity column specification>`
 */
enum class identity_generation_type {
    /// @brief ignores definition for absent elements.
    always,
    /// @brief failure if other elements refer to the drop target (excludes with cascade).
    by_default,
};

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(identity_generation_type value) noexcept {
    using namespace std::string_view_literals;
    using kind = identity_generation_type;
    switch (value) {
        case kind::always: return "always"sv;
        case kind::by_default: return "by_default"sv;
    }
    std::abort();
}

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
inline std::ostream& operator<<(std::ostream& out, identity_generation_type value) {
    return out << to_string_view(value);
}

} // namespace mizugaki::ast::statement
