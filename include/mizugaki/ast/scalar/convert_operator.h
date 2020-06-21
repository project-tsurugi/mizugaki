#pragma once

#include <ostream>
#include <string>
#include <string_view>

#include <takatori/util/enum_set.h>

namespace mizugaki::ast::scalar {

/**
 * @brief represents a conversion operator.
 * @note `6.18 <string value function>`
 */
enum class convert_operator {
    /**
     * @brief `COVERT`
     * @note `6.18 <string value function>` - `<form-of-use conversion>`
     */
    convert,
    /**
     * @brief `TRANSLATE`
     * @note `6.18 <string value function>` - `<character translation>`
     */
    translate,
};

/// @brief set of convert_operator kind.
using convert_operator_set = ::takatori::util::enum_set<
        convert_operator,
        convert_operator::convert,
        convert_operator::translate>;

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(convert_operator value) noexcept {
    using namespace std::string_view_literals;
    using kind = convert_operator;
    switch (value) {
        case kind::convert: return "convert"sv;
        case kind::translate: return "translate"sv;
    }
    std::abort();
}

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
inline std::ostream& operator<<(std::ostream& out, convert_operator value) {
    return out << to_string_view(value);
}

} // namespace mizugaki::ast::scalar
