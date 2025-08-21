#pragma once

#include <ostream>
#include <string_view>

namespace mizugaki::ast::statement {

/**
 * @brief represents object kind of privileges.
 * @note `10.5 <privileges> - <object name>`
 */
enum class privilege_object_kind {
    /// @brief about table privileges.
    table,

    /// @brief about schema privileges.
    schema,
};

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(privilege_object_kind value) noexcept {
    using namespace std::string_view_literals;
    using kind = privilege_object_kind;
    switch (value) {
        case kind::table: return "table"sv;
        case kind::schema: return "schema"sv;
    }
    std::abort();
}

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
inline std::ostream& operator<<(std::ostream& out, privilege_object_kind value) {
    return out << to_string_view(value);
}

} // namespace mizugaki::ast::statement
