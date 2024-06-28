#pragma once

#include <ostream>
#include <string>
#include <string_view>

namespace mizugaki::ast::statement {

/**
 * @brief represents an option of drop_statement.
 * @see `11.2 <drop schema statement>` - `<drop behavior>`
 */
enum class drop_statement_option {
    /// @brief ignores definition for absent elements.
    if_exists,
    /// @brief failure if other elements refer to the drop target (excludes with cascade).
    restrict,
    /// @brief also delete if other elements refer to the drop target (excludes with restrict).
    cascade,
};

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(drop_statement_option value) noexcept {
    using namespace std::string_view_literals;
    using kind = drop_statement_option;
    switch (value) {
        case kind::if_exists: return "if_exists"sv;
        case kind::restrict: return "restrict"sv;
        case kind::cascade: return "cascade"sv;
    }
    std::abort();
}

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
inline std::ostream& operator<<(std::ostream& out, drop_statement_option value) {
    return out << to_string_view(value);
}

} // namespace mizugaki::ast::statement
