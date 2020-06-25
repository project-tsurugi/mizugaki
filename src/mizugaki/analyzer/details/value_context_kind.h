#pragma once

#include <ostream>
#include <string>
#include <string_view>

namespace mizugaki::analyzer::details {

enum class value_context_kind : std::size_t {
    unspecified = 0,
    scalar,
    row,
};

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(value_context_kind value) noexcept {
    using namespace std::string_view_literals;
    using kind = value_context_kind;
    switch (value) {
        case kind::unspecified: return "unspecified"sv;
        case kind::scalar: return "scalar"sv;
        case kind::row: return "row"sv;
    }
    std::abort();
}

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
inline std::ostream& operator<<(std::ostream& out, value_context_kind value) {
    return out << to_string_view(value);
}

} // namespace mizugaki::analyzer::details
