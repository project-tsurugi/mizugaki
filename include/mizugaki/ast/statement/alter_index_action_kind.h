#pragma once

#include <ostream>
#include <string_view>

namespace mizugaki::ast::statement {

/**
 * @brief represents action kind of alter index statement.
 * @note This is out of SQL standard
 * @see alter_index_statement
 */
enum class alter_index_action_kind {
    /**
     * @brief `ALTER INDEX ... RENAME TO ...`.
     */
    rename_index,
};

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
constexpr std::string_view to_string_view(alter_index_action_kind value) noexcept {
    using namespace std::string_view_literals;
    using kind = alter_index_action_kind;
    switch (value) {
        case kind::rename_index: return "rename_index"sv;
    }
    std::abort();
}

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
inline std::ostream& operator<<(std::ostream& out, alter_index_action_kind value) {
    return out << to_string_view(value);
}

} // namespace mizugaki::ast::statement
