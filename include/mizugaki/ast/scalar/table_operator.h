#pragma once

#include <ostream>
#include <string>
#include <string_view>

#include <takatori/util/enum_set.h>

namespace mizugaki::ast::scalar {

/**
 * @brief represents a unary operator .
 */
enum class table_operator {
    /**
     * @brief test for a non-empty relation.
     * @details The operand must be a subquery, and it will be treated as a table sub-query.
     * @note `8.9 <exists predicate>`
     */
    exists,

    /**
     * @brief test for absence of duplications.
     * @details The operand must be a subquery, and it will be treated as a table sub-query.
     * @note `8.10 <unique predicate>`
     */
    unique,
};

/// @brief set of table_operator kind.
using table_operator_set = ::takatori::util::enum_set<
        table_operator,
        table_operator::exists,
        table_operator::unique>;

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(table_operator value) noexcept {
    using namespace std::string_view_literals;
    using kind = table_operator;
    switch (value) {
        case kind::exists: return "exists"sv;
        case kind::unique: return "unique"sv;
    }
    std::abort();
}

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
inline std::ostream& operator<<(std::ostream& out, table_operator value) {
    return out << to_string_view(value);
}

} // namespace mizugaki::ast::scalar
