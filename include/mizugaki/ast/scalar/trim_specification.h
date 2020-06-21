#pragma once

#include <ostream>
#include <string>
#include <string_view>

#include <takatori/util/enum_set.h>
#include <takatori/util/meta_type.h>

namespace mizugaki::ast::scalar {

/**
 * @brief represents a kind of built-in function.
 * @note `6.18 <string value function>` - `<trim specification>`
 */
enum class trim_specification {
    /// @brief `LEADING`
    leading,
    /// @brief `TRAILING`
    trailing,
    /// @brief `BOTH`
    both,
};

/// @brief set of trim_specification kind.
using trim_specification_set = ::takatori::util::enum_set<
        trim_specification,
        trim_specification::leading,
        trim_specification::both>;

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(trim_specification value) noexcept {
    using namespace std::string_view_literals;
    using kind = trim_specification;
    switch (value) {
        case kind::leading: return "leading"sv;
        case kind::trailing: return "trailing"sv;
        case kind::both: return "both"sv;
    }
    std::abort();
}

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
inline std::ostream& operator<<(std::ostream& out, trim_specification value) {
    return out << to_string_view(value);
}

} // namespace mizugaki::ast::scalar
