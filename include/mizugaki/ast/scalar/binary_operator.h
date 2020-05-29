#pragma once

#include <ostream>
#include <string>
#include <string_view>

#include <takatori/util/enum_set.h>

namespace mizugaki::ast::scalar {

/**
 * @brief represents a binary operator.
 */
enum class binary_operator {
    /**
     * @brief plus sign (`+`).
     * @note `6.26 <numeric value expression>`
     * @note `6.28 <datetime value expression>`
     * @note `6.29 <interval value expression>`
     */
    plus,

    /**
     * @brief minus sign (`-`).
     * @note `6.26 <numeric value expression>`
     * @note `6.28 <datetime value expression>`
     * @note `6.29 <interval value expression>`
     */
    minus,

    /**
     * @brief asterisk (`*`).
     * @note `6.26 <numeric value expression>`
     * @note `6.29 <interval value expression>`
     */
    asterisk,

    /**
     * @brief slash (`/`).
     * @note `6.26 <numeric value expression>`
     * @note `6.29 <interval value expression>`
     */
    solidus,

    /**
     * @brief concatenation operator (`||`).
     * @note `6.27 <string value expression>`
     * @note `6.31 <array value expression>`
     */
    concatenation,

    /**
     * @brief conditional `AND`.
     * @note `6.30 <boolean value expression>`
     */
    and_,

    /**
     * @brief conditional `OR`.
     * @note `6.30 <boolean value expression>`
     */
    or_,

    /**
     * @brief test for a boolean or null value.
     * @details The right term must be literal_expression with boolean value or null.
     * @note `6.30 <boolean value expression>`
     * @note `8.7 <null predicate>`
     * @see literal_expression
     */
    is,

    /// @copydoc binary_operator::is
    is_not,

    /**
     * @brief quantified comparison with sub-queries.
     * @details The right term must be a subquery, and it will be treated as a table sub-query.
     *      Use in_predicate expression for `IN` predicate with value list instead.
     * @note `8.4 <in predicate>`
     * @see subquery
     * @see in_predicate
     */
    in,

    /// @copydoc binary_operator::in
    not_in,

    /**
     * @brief test for an overlap between two ranges.
     * @note `8.12 <overlaps predicate>`
     */
    overlaps,

    /**
     * @brief test for two rows are distinct.
     * @note `8.13 <distinct predicate>`
     */
    is_distinct_from,
};

/// @brief set of binary_operator kind.
using binary_operator_set = ::takatori::util::enum_set<
        binary_operator,
        binary_operator::plus,
        binary_operator::is_distinct_from>;

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(binary_operator value) noexcept {
    using namespace std::string_view_literals;
    using kind = binary_operator;
    switch (value) {
        case kind::plus: return "plus"sv;
        case kind::minus: return "minus"sv;
        case kind::asterisk: return "asterisk"sv;
        case kind::solidus: return "solidus"sv;
        case kind::concatenation: return "concatenation"sv;
        case kind::and_: return "and"sv;
        case kind::or_: return "or"sv;
        case kind::is: return "is"sv;
        case kind::is_not: return "is_not"sv;
        case kind::in: return "in"sv;
        case kind::not_in: return "not_in"sv;
        case kind::overlaps: return "overlaps"sv;
        case kind::is_distinct_from: return "is_distinct_from"sv;
    }
    std::abort();
}

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
inline std::ostream& operator<<(std::ostream& out, binary_operator value) {
    return out << to_string_view(value);
}

} // namespace mizugaki::ast::scalar
