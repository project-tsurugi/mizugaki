#pragma once

#include <ostream>
#include <string>
#include <string_view>

namespace mizugaki::ast::statement {

/**
 * @brief represents an option of sequence_definition.
 */
enum class sequence_definition_option {
    /// @brief enable value overflow (wrap rounding).
    cycle,
    /// @brief explicitly disables value overflow (wrap rounding).
    no_cycle,
    /// @brief the sequence is temporary in the current transaction.
    temporary,
    /// @brief ignores definition for existing tables.
    if_not_exists,
};

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(sequence_definition_option value) noexcept {
    using namespace std::string_view_literals;
    using kind = sequence_definition_option;
    switch (value) {
        case kind::cycle: return "cycle"sv;
        case kind::no_cycle: return "no_cycle"sv;
        case kind::temporary: return "temporary"sv;
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
inline std::ostream& operator<<(std::ostream& out, sequence_definition_option value) {
    return out << to_string_view(value);
}

} // namespace mizugaki::ast::statement
