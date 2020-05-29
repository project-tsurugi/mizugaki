#pragma once

#include <ostream>
#include <string>
#include <string_view>

#include <takatori/util/enum_set.h>
#include <takatori/util/meta_type.h>

namespace mizugaki::ast::scalar {

/**
 * @brief represents a comparison operator.
 * @note `8.2 <comparison predicate>`
 * @note `8.8 <quantified comparison predicate>`
 * @see quantifier
 */
enum class comparison_operator {
    /// @brief `=`
    equals,
    /// @brief `<>`
    not_equals,
    /// @brief `<`
    less_than,
    /// @brief `>`
    greater_than,
    /// @brief `<=`
    less_than_or_equals,
    /// @brief `>=`
    greater_than_or_equals,
};

/// @brief set of comparison_operator kind.
using comparison_operator_set = ::takatori::util::enum_set<
        comparison_operator,
        comparison_operator::equals,
        comparison_operator::greater_than_or_equals>;

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(comparison_operator value) noexcept {
    using namespace std::string_view_literals;
    using kind = comparison_operator;
    switch (value) {
        case kind::equals: return "equals"sv;
        case kind::not_equals: return "not_equals"sv;
        case kind::less_than: return "less_than"sv;
        case kind::greater_than: return "greater_than"sv;
        case kind::less_than_or_equals: return "less_than_or_equals"sv;
        case kind::greater_than_or_equals: return "greater_than_or_equals"sv;
    }
    std::abort();
}

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
inline std::ostream& operator<<(std::ostream& out, comparison_operator value) {
    return out << to_string_view(value);
}

} // namespace mizugaki::ast::scalar
