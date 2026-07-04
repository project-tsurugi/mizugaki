#pragma once

#include <ostream>
#include <string_view>

namespace mizugaki::ast::statement {

/**
 * @brief represents action kind of alter table statement.
 * @note `11.10 <alter table statement> - <alter table action>`
 * @see alter_table_statement
 */
enum class alter_table_action_kind {
    /**
     * @brief `ALTER TABLE ... RENAME TO ...`.
     */
    rename_table,

    /**
     * @brief `ALTER TABLE ... RENAME COLUMN ... TO ...`.
     */
    rename_column,
};

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
constexpr std::string_view to_string_view(alter_table_action_kind value) noexcept {
    using namespace std::string_view_literals;
    using kind = alter_table_action_kind;
    switch (value) {
        case kind::rename_table: return "rename_table"sv;
        case kind::rename_column: return "rename_column"sv;
    }
    std::abort();
}

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
inline std::ostream& operator<<(std::ostream& out, alter_table_action_kind value) {
    return out << to_string_view(value);
}

} // namespace mizugaki::ast::statement
