#pragma once

#include <ostream>
#include <string>
#include <string_view>
#include <utility>

#include <takatori/util/enum_set.h>
#include <takatori/util/meta_type.h>

namespace mizugaki::ast::query {

class grouping_column;

/**
 * @brief represents a kind of grouping element.
 * @note `7.9 <group by clause>` - `<grouping specification>`
 */
enum class grouping_element_kind {
    /**
     * @copydoc select_column
     * @see select_column
     */
    column,

    // FIXME: more
};

/// @brief set of grouping element kind.
using grouping_element_kind_set = ::takatori::util::enum_set<
        grouping_element_kind,
        grouping_element_kind::column,
        grouping_element_kind::column>; // FIXME

template<class K, K Kind> struct type_of;

template<auto Kind> using type_of_t = typename type_of<decltype(Kind), Kind>::type;

/// @brief provides implementation type of grouping_element_kind::column.
template<> struct type_of<grouping_element_kind, grouping_element_kind::column> : ::takatori::util::meta_type<grouping_column> {};

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(grouping_element_kind value) noexcept {
    using namespace std::string_view_literals;
    using kind = grouping_element_kind;
    switch (value) {
        case kind::column: return "column"sv;
    }
    std::abort();
}

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
inline std::ostream& operator<<(std::ostream& out, grouping_element_kind value) {
    return out << to_string_view(value);
}

} // namespace mizugaki::ast::query
