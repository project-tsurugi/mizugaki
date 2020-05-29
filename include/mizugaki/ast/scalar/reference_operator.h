#pragma once

#include <ostream>
#include <string>
#include <string_view>

#include <takatori/util/enum_set.h>

namespace mizugaki::ast::scalar {

/**
 * @brief represents a reference operator.
 */
enum class reference_operator {
    /**
     * @brief data conversion.
     * @note `6.8 <field reference>`
     * @note `6.11 <method invocation>`
     */
    period,

    /**
     * @brief dereference operator.
     * @note `6.9 <attribute or method reference>`
     * @note `6.10 <method reference>`
     */
    arrow,
};

/// @brief set of reference_operator kind.
using reference_operator_set = ::takatori::util::enum_set<
        reference_operator,
        reference_operator::period,
        reference_operator::arrow>;

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(reference_operator value) noexcept {
    using namespace std::string_view_literals;
    using kind = reference_operator;
    switch (value) {
        case kind::period: return "period"sv;
        case kind::arrow: return "arrow"sv;
    }
    std::abort();
}

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
inline std::ostream& operator<<(std::ostream& out, reference_operator value) {
    return out << to_string_view(value);
}

} // namespace mizugaki::ast::scalar
