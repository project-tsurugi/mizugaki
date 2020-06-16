#pragma once

#include <ostream>
#include <string>
#include <string_view>
#include <utility>

#include <takatori/util/enum_set.h>
#include <takatori/util/meta_type.h>

namespace mizugaki::ast::table {

class join_condition;
class join_columns;

/**
 * @brief represents a kind of join specification.
 * @note `7.7 <joined table>` - `<join specification>`
 */
enum class join_specification_kind {
    /**
     * @copydoc join_condition
     * @see join_condition
     */
    condition,

    /**
     * @copydoc join_columns
     * @see join_columns
     */
    columns,
};

/// @brief set of grouping element kind.
using join_specification_kind_set = ::takatori::util::enum_set<
        join_specification_kind,
        join_specification_kind::condition,
        join_specification_kind::columns>;

template<class K, K Kind> struct type_of;

template<auto Kind> using type_of_t = typename type_of<decltype(Kind), Kind>::type;

/// @brief provides implementation type of join_specification_kind::condition.
template<> struct type_of<join_specification_kind, join_specification_kind::condition> : ::takatori::util::meta_type<join_condition> {};

/// @brief provides implementation type of join_specification_kind::columns.
template<> struct type_of<join_specification_kind, join_specification_kind::columns> : ::takatori::util::meta_type<join_columns> {};

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(join_specification_kind value) noexcept {
    using namespace std::string_view_literals;
    using kind = join_specification_kind;
    switch (value) {
        case kind::condition: return "condition"sv;
        case kind::columns: return "columns"sv;
    }
    std::abort();
}

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
inline std::ostream& operator<<(std::ostream& out, join_specification_kind value) {
    return out << to_string_view(value);
}

} // namespace mizugaki::ast::table
