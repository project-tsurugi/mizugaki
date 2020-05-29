#pragma once

#include <ostream>
#include <string>
#include <string_view>

#include <takatori/util/enum_set.h>

namespace mizugaki::ast::common {

/**
 * @brief represents a sort direction.
 * @note `14.1 <declare cursor>`
 */
enum class ordering_specification {
    /// @brief `ASC`
    asc,
    /// @brief `DESC`.
    desc,
};

/// @brief set of ordering specification.
using ordering_specification_set = ::takatori::util::enum_set<
        ordering_specification,
        ordering_specification::asc,
        ordering_specification::desc>;

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(ordering_specification value) noexcept {
    using namespace std::string_view_literals;
    switch (value) {
        case ordering_specification::asc: return "asc"sv;
        case ordering_specification::desc: return "desc"sv;
    }
    std::abort();
}

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
inline std::ostream& operator<<(std::ostream& out, ordering_specification value) {
    return out << to_string_view(value);
}

} // namespace mizugaki::ast::common
