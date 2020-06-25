#pragma once

#include <ostream>
#include <string>
#include <string_view>

namespace mizugaki::ast::statement {

/**
 * @brief represents an option of index_definition.
 */
enum class index_definition_option {
    /// @brief require unique key.
    unique,
    /// @brief ignores definition for existing indices.
    if_not_exists,
};

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(index_definition_option value) noexcept {
    using namespace std::string_view_literals;
    using kind = index_definition_option;
    switch (value) {
        case kind::unique: return "unique"sv;
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
inline std::ostream& operator<<(std::ostream& out, index_definition_option value) {
    return out << to_string_view(value);
}

} // namespace mizugaki::ast::statement
