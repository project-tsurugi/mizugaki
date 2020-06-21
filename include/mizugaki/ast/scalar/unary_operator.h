#pragma once

#include <ostream>
#include <string>
#include <string_view>

#include <takatori/util/enum_set.h>

namespace mizugaki::ast::scalar {

/**
 * @brief represents a unary operator.
 */
enum class unary_operator {
    /**
     * @brief plus sign (`+`).
     * @note `6.26 <numeric value expression>`
     * @note `6.29 <interval value expression>`
     */
    plus,

    /**
     * @brief minus sign (`+`).
     * @note `6.26 <numeric value expression>`
     * @note `6.29 <interval value expression>`
     */
    minus,

    /**
     * @brief conditional `NOT`.
     * @note `6.30 <boolean value expression>`
     */
    not_,

    /**
     * @brief reference resolution (`DEREF`).
     * @note `6.15 <reference resolution>`
     */
    reference_resolution,
};

/// @brief set of unary_operator kind.
using unary_operator_set = ::takatori::util::enum_set<
        unary_operator,
        unary_operator::plus,
        unary_operator::reference_resolution>;

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(unary_operator value) noexcept {
    using namespace std::string_view_literals;
    using kind = unary_operator;
    switch (value) {
        case kind::plus: return "plus"sv;
        case kind::minus: return "minus"sv;
        case kind::not_: return "not"sv;
        case kind::reference_resolution: return "reference_resolution"sv;
    }
    std::abort();
}

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
inline std::ostream& operator<<(std::ostream& out, unary_operator value) {
    return out << to_string_view(value);
}

} // namespace mizugaki::ast::scalar
