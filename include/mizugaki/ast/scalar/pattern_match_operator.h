#pragma once

#include <ostream>
#include <string>
#include <string_view>

#include <takatori/util/enum_set.h>

namespace mizugaki::ast::scalar {

/**
 * @brief represents a pattern-match operator.
 * @note `8.5 <like predicate>`
 */
enum class pattern_match_operator {
    /**
     * @brief performs simple pattern matching.
     * @note `8.5 <like predicate>`
     */
    like,

    /**
     * @brief performs regular expression matching.
     * @note `8.6 <similar predicate>`
     */
    similar_to,
};

/// @brief set of pattern_match_operator kind.
using pattern_match_operator_set = ::takatori::util::enum_set<
        pattern_match_operator,
        pattern_match_operator::like,
        pattern_match_operator::similar_to>;

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(pattern_match_operator value) noexcept {
    using namespace std::string_view_literals;
    using kind = pattern_match_operator;
    switch (value) {
        case kind::like: return "like"sv;
        case kind::similar_to: return "similar_to"sv;
    }
    std::abort();
}

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
inline std::ostream& operator<<(std::ostream& out, pattern_match_operator value) {
    return out << to_string_view(value);
}

} // namespace mizugaki::ast::scalar
