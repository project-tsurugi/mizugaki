#pragma once

#include <ostream>
#include <string>
#include <string_view>

#include <takatori/util/enum_set.h>

namespace mizugaki::ast::statement {

/**
 * @brief represents an option of insert_statement.
 */
enum class insert_statement_option {
    /// @brief replaces the entry if insertion value violates a unique constraint.
    or_replace,
    /// @brief skips the entry if insertion value violates a unique constraint.
    or_ignore,
};

/// @brief set of table option kind.
using insert_statement_option_set = ::takatori::util::enum_set<
        insert_statement_option,
        insert_statement_option::or_replace,
        insert_statement_option::or_ignore>;

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(insert_statement_option value) noexcept {
    using namespace std::string_view_literals;
    using kind = insert_statement_option;
    switch (value) {
        case kind::or_replace: return "or_replace"sv;
        case kind::or_ignore: return "or_ignore"sv;
    }
    std::abort();
}

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
inline std::ostream& operator<<(std::ostream& out, insert_statement_option value) {
    return out << to_string_view(value);
}

} // namespace mizugaki::ast::statement
