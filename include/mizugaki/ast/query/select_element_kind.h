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
class select_all_fields;

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

    /**
     * @copydoc select_all_fields
     * @see select_all_fields
     */
    all_fields,
};

/// @brief set of select element kind.
using select_element_kind_set = ::takatori::util::enum_set<
        select_element_kind,
        select_element_kind::column,
        select_element_kind::all_fields>;

/**
 * @brief provides the implementation type of the select element.
 * @tparam Kind the value kind
 */
template<select_element_kind Kind> struct select_element_of;

/// @copydoc select_element_of
template<select_element_kind Kind> using select_element_of_t = typename select_element_of<Kind>::type;

/// @brief provides implementation type of select_element_kind::column.
template<> struct select_element_of<select_element_kind::column> : ::takatori::util::meta_type<select_column> {};

/// @brief provides implementation type of select_element_kind::asterisk.
template<> struct select_element_of<select_element_kind::asterisk> : ::takatori::util::meta_type<select_asterisk> {};

/// @brief provides implementation type of select_element_kind::all_fields.
template<> struct select_element_of<select_element_kind::all_fields> : ::takatori::util::meta_type<select_all_fields> {};

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
        case kind::all_fields: return "all_fields"sv;
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
