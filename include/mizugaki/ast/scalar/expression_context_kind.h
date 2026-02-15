#pragma once

#include <ostream>
#include <string>
#include <string_view>

namespace mizugaki::ast::scalar {

/**
 * @brief represents a kind of expression context.
 * @note `7.14 <subquery>`
 * @see subquery
 */
enum class expression_context_kind {
    /**
     * @brief scalar context.
     */
    scalar,

    /**
     * @brief row context.
     */
    row,
};

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
constexpr std::string_view to_string_view(expression_context_kind value) noexcept {
    using namespace std::string_view_literals;
    using kind = expression_context_kind;
    switch (value) {
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
inline std::ostream& operator<<(std::ostream& out, expression_context_kind value) {
    return out << to_string_view(value);
}

} // namespace mizugaki::ast::scalar
