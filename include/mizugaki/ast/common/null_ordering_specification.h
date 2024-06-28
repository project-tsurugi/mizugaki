#pragma once

#include <ostream>
#include <string>
#include <string_view>

#include <takatori/util/enum_set.h>

namespace mizugaki::ast::common {

/**
 * @brief represents location of nulls.
 * @note `14.1 <declare cursor>`
 */
enum class null_ordering_specification {
    /// @brief `NULLS FIRST`
    first,
    /// @brief `NULLS LAST`.
    last,
};

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(null_ordering_specification value) noexcept {
    using namespace std::string_view_literals;
    switch (value) {
        case null_ordering_specification::first: return "first"sv;
        case null_ordering_specification::last: return "last"sv;
    }
    std::abort();
}

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
inline std::ostream& operator<<(std::ostream& out, null_ordering_specification value) {
    return out << to_string_view(value);
}

} // namespace mizugaki::ast::common
