#pragma once

#include <ostream>
#include <string>
#include <string_view>

#include <takatori/util/enum_set.h>
#include <takatori/util/meta_type.h>

namespace mizugaki::ast::scalar {

/**
 * @brief represents a quantifier.
 * @note `8.8 <quantified comparison predicate>`
 */
enum class quantifier {
    /// @brief `ALL`
    all,
    /// @brief `SOME`
    some,
    /// @brief `ANY`
    any,
};

/// @brief set of quantifier kind.
using quantifier_set = ::takatori::util::enum_set<
        quantifier,
        quantifier::all,
        quantifier::any>;

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(quantifier value) noexcept {
    using namespace std::string_view_literals;
    using kind = quantifier;
    switch (value) {
        case kind::all: return "all"sv;
        case kind::some: return "some"sv;
        case kind::any: return "any"sv;
    }
    std::abort();
}

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
inline std::ostream& operator<<(std::ostream& out, quantifier value) {
    return out << to_string_view(value);
}

} // namespace mizugaki::ast::scalar
