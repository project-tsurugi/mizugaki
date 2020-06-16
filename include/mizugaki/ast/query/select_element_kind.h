#pragma once

#include <ostream>
#include <string>
#include <string_view>
#include <utility>

#include <takatori/util/enum_set.h>
#include <takatori/util/meta_type.h>

namespace mizugaki::ast::query {

class select_column;
class select_asterisk;

/**
 * @brief represents a kind of select element.
 * @note `7.11 <query specification>` - `<select list>`
 */
enum class select_element_kind {
    /**
     * @copydoc select_column
     * @see select_column
     */
    column,

    /**
     * @copydoc select_asterisk
     * @see select_asterisk
     */
    asterisk,
};

/// @brief set of select element kind.
using select_element_kind_set = ::takatori::util::enum_set<
        select_element_kind,
        select_element_kind::column,
        select_element_kind::asterisk>;

template<class K, K Kind> struct type_of;

template<auto Kind> using type_of_t = typename type_of<decltype(Kind), Kind>::type;

/// @brief provides implementation type of select_element_kind::column.
template<> struct type_of<select_element_kind, select_element_kind::column> : ::takatori::util::meta_type<select_column> {};

/// @brief provides implementation type of select_element_kind::asterisk.
template<> struct type_of<select_element_kind, select_element_kind::asterisk> : ::takatori::util::meta_type<select_asterisk> {};

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(select_element_kind value) noexcept {
    using namespace std::string_view_literals;
    using kind = select_element_kind;
    switch (value) {
        case kind::column: return "column"sv;
        case kind::asterisk: return "asterisk"sv;
    }
    std::abort();
}

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
inline std::ostream& operator<<(std::ostream& out, select_element_kind value) {
    return out << to_string_view(value);
}

} // namespace mizugaki::ast::query
