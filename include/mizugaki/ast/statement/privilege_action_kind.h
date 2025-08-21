#pragma once

#include <ostream>
#include <string_view>

namespace mizugaki::ast::statement {

/**
 * @brief represents action kind of privileges.
 * @note `10.5 <privileges> - <action>`
 */
enum class privilege_action_kind {
    /// @brief pseudo action kind of all privileges.
    all_privileges,

    /// @brief action kind of reading table entries.
    select,

    /// @brief action kind of creating table entries.
    insert,

    /// @brief action kind of updating table entries.
    update,

    /// @brief action kind of deleting table entries.
    delete_,
};

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(privilege_action_kind value) noexcept {
    using namespace std::string_view_literals;
    using kind = privilege_action_kind;
    switch (value) {
        case kind::all_privileges: return "all_privileges"sv;
        case kind::select: return "select"sv;
        case kind::insert: return "insert"sv;
        case kind::update: return "update"sv;
        case kind::delete_: return "delete"sv;
    }
    std::abort();
}

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
inline std::ostream& operator<<(std::ostream& out, privilege_action_kind value) {
    return out << to_string_view(value);
}

} // namespace mizugaki::ast::statement
