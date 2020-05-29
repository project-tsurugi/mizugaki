#pragma once

#include <ostream>
#include <string>
#include <string_view>

#include <takatori/util/enum_set.h>

namespace mizugaki::ast::query {

/**
 * @brief represents a binary operator of query expressions.
 * @note `7.12 <query expression>`
 */
enum class binary_operator {
    /**
     * @brief `UNION`
     */
    union_,

    /**
     * @brief `OUTER UNION`
     */
    outer_union,

    /**
     * @brief `EXCEPT`
     */
    except,

    /**
     * @brief `INTERSECT`
     */
    intersect,
};

/// @brief set of binary_operator kind.
using binary_operator_set = ::takatori::util::enum_set<
        binary_operator,
        binary_operator::union_,
        binary_operator::intersect>;

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(binary_operator value) noexcept {
    using namespace std::string_view_literals;
    using kind = binary_operator;
    switch (value) {
        case kind::union_: return "union"sv;
        case kind::outer_union: return "outer_union"sv;
        case kind::except: return "except"sv;
        case kind::intersect: return "intersect"sv;
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

} // namespace mizugaki::ast::query
