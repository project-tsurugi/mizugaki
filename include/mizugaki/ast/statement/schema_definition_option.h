#pragma once

#include <ostream>
#include <string>
#include <string_view>

namespace mizugaki::ast::statement {

/**
 * @brief represents an option of schema_definition.
 * @note `11.1 <schema definition>`
 */
enum class schema_definition_option {
    /// @brief ignores definition for existing tables.
    if_not_exists,
};

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(schema_definition_option value) noexcept {
    using namespace std::string_view_literals;
    using kind = schema_definition_option;
    switch (value) {
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
inline std::ostream& operator<<(std::ostream& out, schema_definition_option value) {
    return out << to_string_view(value);
}

} // namespace mizugaki::ast::statement
