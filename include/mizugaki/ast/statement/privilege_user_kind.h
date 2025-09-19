#pragma once

#include <ostream>
#include <string_view>

namespace mizugaki::ast::statement {

/**
 * @brief represents action kind of privileges.
 * @note `10.5 <privileges> - <action>`
 */
enum class privilege_user_kind {

    /**
     * @brief distinguish the user by its identifier.
     */
    identifier,

    /**
     * @brief all users except the current user.
     */
    all_users,

    /**
     * @brief the current user.
     */
    current_user,

    /**
     * @brief pseudo user kind for control default privileges.
     */
    public_,
};

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(privilege_user_kind value) noexcept {
    using namespace std::string_view_literals;
    using kind = privilege_user_kind;
    switch (value) {
        case kind::identifier: return "identifier"sv;
        case kind::all_users: return "all_users"sv;
        case kind::current_user: return "current_user"sv;
        case kind::public_: return "public"sv;
    }
    std::abort();
}

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
inline std::ostream& operator<<(std::ostream& out, privilege_user_kind value) {
    return out << to_string_view(value);
}

} // namespace mizugaki::ast::statement
