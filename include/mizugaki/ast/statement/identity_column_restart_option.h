#pragma once

#include <ostream>
#include <string>
#include <string_view>

namespace mizugaki::ast::statement {

/**
 * @brief represents an option of truncate_table_statement.
 */
enum class identity_column_restart_option {
    /// @brief continues identity values over truncation.
    continue_identity,

    /// @brief resets identity values from the initial value.
    restart_identity,
};

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
constexpr std::string_view to_string_view(identity_column_restart_option value) noexcept {
    using namespace std::string_view_literals;
    using kind = identity_column_restart_option;
    switch (value) {
        case kind::continue_identity: return "continue_identity"sv;
        case kind::restart_identity: return "restart_identity"sv;
    }
    std::abort();
}

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, identity_column_restart_option value);

} // namespace mizugaki::ast::statement
