#pragma once

#include <ostream>
#include <string>
#include <string_view>

#include <cstdlib>

namespace mizugaki::ast::statement {

/**
 * @brief represents an option of foreign_catalog_definition.
 */
enum class foreign_catalog_definition_option {
    /// @brief does nothing if the catalog already exists (`IF NOT EXISTS` of the catalog).
    if_not_exists,

    /// @brief does nothing if the server does not exist (`IF EXISTS` of the server).
    if_server_exists,
};

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(foreign_catalog_definition_option value) noexcept {
    using namespace std::string_view_literals;
    using kind = foreign_catalog_definition_option;
    switch (value) {
        case kind::if_not_exists: return "if_not_exists"sv;
        case kind::if_server_exists: return "if_server_exists"sv;
    }
    std::abort();
}

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
inline std::ostream& operator<<(std::ostream& out, foreign_catalog_definition_option value) {
    return out << to_string_view(value);
}

} // namespace mizugaki::ast::statement
