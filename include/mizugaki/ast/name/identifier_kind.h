#pragma once

#include <ostream>
#include <string>
#include <string_view>

#include <takatori/util/meta_type.h>

namespace mizugaki::ast::name {

/**
 * @brief represents a kind of identifier.
 */
enum class identifier_kind {
    /**
     * @brief regular (bare) identifier.
     */
    regular,

    /**
     * @brief delimited identifier.
     */
    delimited,
};

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(identifier_kind value) noexcept {
    using namespace std::string_view_literals;
    switch (value) {
        case identifier_kind::regular: return "regular"sv;
        case identifier_kind::delimited: return "delimited"sv;
    }
    std::abort();
}

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
inline std::ostream& operator<<(std::ostream& out, identifier_kind value) {
    return out << to_string_view(value);
}

} // namespace mizugaki::ast::name
