#pragma once

#include <ostream>
#include <string>
#include <string_view>

#include <takatori/util/enum_set.h>
#include <takatori/util/meta_type.h>

namespace mizugaki::ast::table {

class table_reference;
class unnest; // FIXME: impl
class join;
class subquery;

/**
 * @brief represents a kind of table expression.
 * @note `7.6 <table reference>`
 */
enum class kind {
    /**
     * @copydoc table_reference
     * @see table_reference
     */
    table_reference,

    /**
     * @brief unnest collection values.
     * @note `7.6 <table reference>` - `<collection derived table>`
     */
    unnest,

    /**
     * @copydoc join
     * @see join
     */
    join,

    /**
     * @copydoc subquery
     * @see subquery
     */
    subquery,
};

/// @brief set of table expression kind.
using kind_set = ::takatori::util::enum_set<
        kind,
        kind::table_reference,
        kind::subquery>;

/**
 * @brief provides the implementation type of the table expression kind.
 * @tparam Kind the value kind
 */
template<kind Kind> struct type_of;

/// @copydoc type_of
template<kind Kind> using type_of_t = typename type_of<Kind>::type;

/// @brief provides implementation type of kind::table_reference.
template<> struct type_of<kind::table_reference> : ::takatori::util::meta_type<table_reference> {};

/// @brief provides implementation type of kind::unnest.
template<> struct type_of<kind::unnest> : ::takatori::util::meta_type<unnest> {};

/// @brief provides implementation type of kind::join.
template<> struct type_of<kind::join> : ::takatori::util::meta_type<join> {};

/// @brief provides implementation type of kind::subquery.
template<> struct type_of<kind::subquery> : ::takatori::util::meta_type<subquery> {};

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(kind value) noexcept {
    using namespace std::string_view_literals;
    switch (value) {
        case kind::table_reference: return "table_reference"sv;
        case kind::unnest: return "unnest"sv;
        case kind::join: return "join"sv;
        case kind::subquery: return "subquery"sv;
    }
    std::abort();
}

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
inline std::ostream& operator<<(std::ostream& out, kind value) {
    return out << to_string_view(value);
}

} // namespace mizugaki::ast::table
