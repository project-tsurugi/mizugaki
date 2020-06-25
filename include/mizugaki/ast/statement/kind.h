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
class empty_statement;
class table_definition;
class index_definition;
class view_definition;
class sequence_definition;
class schema_definition;
class drop_statement;

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

    /**
     * @brief table_definition
     * @see table_definition
     */
    table_definition,

    /**
     * @brief index_definition
     * @see index_definition
     */
    index_definition,

    /**
     * @brief view_definition
     * @see view_definition
     */
    view_definition,

    /**
     * @brief sequence_definition
     * @see sequence_definition
     */
    sequence_definition,

    /**
     * @brief schema_definition
     * @see schema_definition
     */
    schema_definition,

    /**
     * @brief drop_statement for tables.
     * @see drop_statement
     */
    drop_table_statement,

    /**
     * @brief drop_statement for indices.
     * @see drop_statement
     */
    drop_index_statement,

    /**
     * @brief drop_statement for indices.
     * @see drop_statement
     */
    drop_view_statement,

    /**
     * @brief drop_statement for sequences.
     * @see drop_statement
     */
    drop_sequence_statement,

    /**
     * @brief drop_statement for indices.
     * @see drop_statement
     */
    drop_schema_statement,

    /**
     * @copydoc empty_statement
     * @see empty_statement
     */
    empty_statement,
};

/// @brief set of statement kind.
using kind_set = ::takatori::util::enum_set<
        kind,
        kind::select_statement,
        kind::empty_statement>;

/**
 * @brief provides the implementation type of the kind.
 * @tparam K the kind type
 * @tparam Kind the node kind
 */
template<class K, K Kind> struct type_of;

/**
 * @brief provides the implementation type of the node kind.
 * @tparam Kind the node kind
 */
template<auto Kind> using type_of_t = typename type_of<decltype(Kind), Kind>::type;

/// @brief provides implementation type of kind::select_statement.
template<> struct type_of<kind, kind::select_statement> : ::takatori::util::meta_type<select_statement> {};

/// @brief provides implementation type of kind::insert_statement.
template<> struct type_of<kind, kind::insert_statement> : ::takatori::util::meta_type<insert_statement> {};

/// @brief provides implementation type of kind::update_statement.
template<> struct type_of<kind, kind::update_statement> : ::takatori::util::meta_type<update_statement> {};

/// @brief provides implementation type of kind::delete_statement.
template<> struct type_of<kind, kind::delete_statement> : ::takatori::util::meta_type<delete_statement> {};

/// @brief provides implementation type of kind::table_definition.
template<> struct type_of<kind, kind::table_definition> : ::takatori::util::meta_type<table_definition> {};

/// @brief provides implementation type of kind::index_definition.
template<> struct type_of<kind, kind::index_definition> : ::takatori::util::meta_type<index_definition> {};

/// @brief provides implementation type of kind::view_definition.
template<> struct type_of<kind, kind::view_definition> : ::takatori::util::meta_type<view_definition> {};

/// @brief provides implementation type of kind::sequence_definition.
template<> struct type_of<kind, kind::sequence_definition> : ::takatori::util::meta_type<sequence_definition> {};

/// @brief provides implementation type of kind::schema_definition.
template<> struct type_of<kind, kind::schema_definition> : ::takatori::util::meta_type<schema_definition> {};

/// @brief provides implementation type of kind::drop_table_statement.
template<> struct type_of<kind, kind::drop_table_statement> : ::takatori::util::meta_type<drop_statement> {};

/// @brief provides implementation type of kind::drop_index_statement.
template<> struct type_of<kind, kind::drop_index_statement> : ::takatori::util::meta_type<drop_statement> {};

/// @brief provides implementation type of kind::drop_view_statement.
template<> struct type_of<kind, kind::drop_view_statement> : ::takatori::util::meta_type<drop_statement> {};

/// @brief provides implementation type of kind::drop_sequence_statement.
template<> struct type_of<kind, kind::drop_sequence_statement> : ::takatori::util::meta_type<drop_statement> {};

/// @brief provides implementation type of kind::drop_schema_statement.
template<> struct type_of<kind, kind::drop_schema_statement> : ::takatori::util::meta_type<drop_statement> {};

/// @brief provides implementation type of kind::empty_statement.
template<> struct type_of<kind, kind::empty_statement> : ::takatori::util::meta_type<empty_statement> {};

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
        case kind::table_definition: return "table_definition"sv;
        case kind::index_definition: return "index_definition"sv;
        case kind::view_definition: return "view_definition"sv;
        case kind::sequence_definition: return "sequence_definition"sv;
        case kind::schema_definition: return "schema_definition"sv;
        case kind::drop_table_statement: return "drop_table_statement"sv;
        case kind::drop_index_statement: return "drop_index_statement"sv;
        case kind::drop_view_statement: return "drop_view_statement"sv;
        case kind::drop_sequence_statement: return "drop_sequence_statement"sv;
        case kind::drop_schema_statement: return "drop_schema_statement"sv;
        case kind::empty_statement: return "empty_statement"sv;
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
