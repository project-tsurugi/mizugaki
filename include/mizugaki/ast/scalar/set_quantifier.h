#pragma once

#include <ostream>
#include <string>
#include <string_view>

#include <takatori/util/enum_set.h>

namespace mizugaki::ast::scalar {

/**
 * @brief represents a set quantifier.
 * @note `6.16 <set function specification>`
 */
enum class set_quantifier {
    /// @brief `DISTINCT`
    distinct,
    /// @brief `ALL`
    all,
};

/// @brief set of set_quantifier kind.
using set_quantifier_set = ::takatori::util::enum_set<
        set_quantifier,
        set_quantifier::distinct,
        set_quantifier::all>;

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(set_quantifier value) noexcept {
    using namespace std::string_view_literals;
    using kind = set_quantifier;
    switch (value) {
        case kind::distinct: return "distinct"sv;
        case kind::all: return "all"sv;
    }
    std::abort();
}

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
inline std::ostream& operator<<(std::ostream& out, set_quantifier value) {
    return out << to_string_view(value);
}

} // namespace mizugaki::ast::scalar
