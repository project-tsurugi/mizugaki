#pragma once

#include <ostream>
#include <string>
#include <string_view>

namespace mizugaki::ast::literal {

/**
 * @brief represents a kind of numeric sign.
 */
enum class sign {
    /// @brief plus sign.
    plus,
    /// @brief minus sign.
    minus,
};

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(sign value) noexcept {
    using namespace std::string_view_literals;
    switch (value) {
        case sign::plus: return "plus"sv;
        case sign::minus: return "minus"sv;
    }
    std::abort();
}

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
inline std::ostream& operator<<(std::ostream& out, sign value) {
    return out << to_string_view(value);
}

} // namespace mizugaki::ast::literal
