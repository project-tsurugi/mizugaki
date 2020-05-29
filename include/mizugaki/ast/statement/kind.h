#pragma once

#include <ostream>
#include <string>
#include <string_view>

#include <takatori/util/enum_set.h>
#include <takatori/util/meta_type.h>

namespace mizugaki::ast::statement {

class select_statement;
class insert_statement;
class update_statement;
class delete_statement;

/**
 * @brief represents a kind of SQL statements.
 * @see `13.5 <SQL procedure statement>`
 */
enum class kind {
    /**
     * @copydoc select_statement
     * @see select_statement
     */
    select_statement,

    /**
     * @copydoc insert_statement
     * @see insert_statement
     */
    insert_statement,

    /**
     * @copydoc update_statement
     * @see update_statement
     */
    update_statement,

    /**
     * @copydoc delete_statement
     * @see delete_statement
     */
    delete_statement,

    // FIXME: more
};

/// @brief set of statement kind.
using kind_set = ::takatori::util::enum_set<
        kind,
        kind::select_statement,
        kind::delete_statement>; // FIXME: impl

/**
 * @brief provides the implementation type of the kind.
 * @param Kind the kind type
 * @tparam Kind the scalar expression kind
 */
template<class K, K Kind> struct type_of;

/// @copydoc type_of
template<kind Kind> using type_of_t = typename type_of<decltype(Kind), Kind>::type;

/// @brief provides implementation type of kind::select_statement.
template<> struct type_of<kind, kind::select_statement> : ::takatori::util::meta_type<select_statement> {};

/// @brief provides implementation type of kind::insert_statement.
template<> struct type_of<kind, kind::insert_statement> : ::takatori::util::meta_type<insert_statement> {};

/// @brief provides implementation type of kind::update_statement.
template<> struct type_of<kind, kind::update_statement> : ::takatori::util::meta_type<update_statement> {};

/// @brief provides implementation type of kind::delete_statement.
template<> struct type_of<kind, kind::delete_statement> : ::takatori::util::meta_type<delete_statement> {};

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(kind value) noexcept {
    using namespace std::string_view_literals;
    switch (value) {
        case kind::select_statement: return "select_statement"sv;
        case kind::insert_statement: return "insert_statement"sv;
        case kind::update_statement: return "update_statement"sv;
        case kind::delete_statement: return "delete_statement"sv;
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

} // namespace mizugaki::ast::statement
