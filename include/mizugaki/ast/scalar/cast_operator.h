#pragma once

#include <ostream>
#include <string>
#include <string_view>

#include <takatori/util/enum_set.h>

namespace mizugaki::ast::scalar {

/**
 * @brief represents a cast operator.
 */
enum class cast_operator {
    /**
     * @brief data conversion.
     * @note `6.22 <cast specification>`
     */
    cast,

    /**
     * @brief treat expression as its super-type.
     * @note `6.25 <subtype treatment>`
     */
    treat,

    /**
     * @brief generalize expression as the specified user-defined type.
     * @note `6.11 <method invocation>` - `<generalized invocation>`
     * @note `10.4 <routine invocation>` - `<generalized expression>`
     */
    generalize,
};

/// @brief set of cast_operator kind.
using cast_operator_set = ::takatori::util::enum_set<
        cast_operator,
        cast_operator::cast,
        cast_operator::generalize>;

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(cast_operator value) noexcept {
    using namespace std::string_view_literals;
    using kind = cast_operator;
    switch (value) {
        case kind::cast: return "cast"sv;
        case kind::treat: return "treat"sv;
        case kind::generalize: return "generalize"sv;
    }
    std::abort();
}

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
inline std::ostream& operator<<(std::ostream& out, cast_operator value) {
    return out << to_string_view(value);
}

} // namespace mizugaki::ast::scalar
