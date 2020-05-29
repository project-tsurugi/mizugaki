#pragma once

#include <ostream>
#include <string>
#include <string_view>

#include <takatori/util/enum_set.h>

namespace mizugaki::ast::scalar {

/**
 * @brief represents a between operator.
 */
enum class between_operator {
    /**
     * @brief a-symmetric comparison.
     */
    asymmetric,

    /**
     * @brief symmetric comparison.
     */
    symmetric,
};

/// @brief set of between_operator kind.
using between_operator_set = ::takatori::util::enum_set<
        between_operator,
        between_operator::asymmetric,
        between_operator::symmetric>;

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(between_operator value) noexcept {
    using namespace std::string_view_literals;
    using kind = between_operator;
    switch (value) {
        case kind::asymmetric: return "asymmetric"sv;
        case kind::symmetric: return "symmetric"sv;
    }
    std::abort();
}

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
inline std::ostream& operator<<(std::ostream& out, between_operator value) {
    return out << to_string_view(value);
}

} // namespace mizugaki::ast::scalar
