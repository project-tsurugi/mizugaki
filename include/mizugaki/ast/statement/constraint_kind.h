#pragma once

#include <ostream>
#include <string>
#include <string_view>
#include <utility>

#include <takatori/util/enum_set.h>
#include <takatori/util/meta_type.h>

namespace mizugaki::ast::statement {

class simple_constraint;
class expression_constraint;
class key_constraint;
class referential_constraint;
class identity_constraint;

/**
 * @brief represents a kind of table element.
 * @note We also treat default clause of column definitions as a constraint.
 * @note `11.4 <column definition>` - `<column constraint>`
 * @note `11.4 <column definition>` - `<default clause>`
 * @note `11.6 <table constraint definition>`
 */
enum class constraint_kind {

    /**
     * @brief check constraint.
     * @see expression_constraint
     */
    check,

    /**
     * @brief unique constraint.
     * @see key_constraint
     */
    unique,

    /**
     * @brief declares a primary key.
     * @see key_constraint
     */
    primary_key,

    /**
     * @brief foreign key constraint.
     * @see referential_constraint
     */
    referential,

    /**
     * @brief the target column may or may not be `NULL`.
     * @see simple_constraint
     */
    null,

    /**
     * @brief the target column must NOT be `NULL`.
     * @see simple_constraint
     */
    not_null,

    /**
     * @brief the column default value.
     * @see expression_constraint
     */
    default_clause,

    /**
     * @brief the generated column.
     * @see expression_constraint
     */
    generation_clause,

    /**
     * @brief the target column is set to identity.
     * @see identity_column_constraint
     */
    identity_column,
};

/// @brief set of constraint kind.
using constraint_kind_set = ::takatori::util::enum_set<
        constraint_kind,
        constraint_kind::check,
        constraint_kind::identity_column>;

template<class K, K Kind> struct type_of;

template<auto Kind> using type_of_t = typename type_of<decltype(Kind), Kind>::type;

/// @brief provides implementation type of constraint_kind::check.
template<> struct type_of<constraint_kind, constraint_kind::check> : ::takatori::util::meta_type<expression_constraint> {};

/// @brief provides implementation type of constraint_kind::unique.
template<> struct type_of<constraint_kind, constraint_kind::unique> : ::takatori::util::meta_type<key_constraint> {};

/// @brief provides implementation type of constraint_kind::primary_key.
template<> struct type_of<constraint_kind, constraint_kind::primary_key> : ::takatori::util::meta_type<key_constraint> {};

/// @brief provides implementation type of constraint_kind::referential.
template<> struct type_of<constraint_kind, constraint_kind::referential> : ::takatori::util::meta_type<referential_constraint> {};

/// @brief provides implementation type of constraint_kind::null.
template<> struct type_of<constraint_kind, constraint_kind::null> : ::takatori::util::meta_type<simple_constraint> {};

/// @brief provides implementation type of constraint_kind::not_null.
template<> struct type_of<constraint_kind, constraint_kind::not_null> : ::takatori::util::meta_type<simple_constraint> {};

/// @brief provides implementation type of constraint_kind::default_clause.
template<> struct type_of<constraint_kind, constraint_kind::default_clause> : ::takatori::util::meta_type<expression_constraint> {};

/// @brief provides implementation type of constraint_kind::generation_clause.
template<> struct type_of<constraint_kind, constraint_kind::generation_clause> : ::takatori::util::meta_type<expression_constraint> {};

/// @brief provides implementation type of constraint_kind::identity_column.
template<> struct type_of<constraint_kind, constraint_kind::identity_column> : ::takatori::util::meta_type<identity_constraint> {};

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(constraint_kind value) noexcept {
    using namespace std::string_view_literals;
    using kind = constraint_kind;
    switch (value) {
        case kind::check: return "check"sv;
        case kind::unique: return "unique"sv;
        case kind::primary_key: return "primary_key"sv;
        case kind::referential: return "referential"sv;
        case kind::null: return "null"sv;
        case kind::not_null: return "not_null"sv;
        case kind::default_clause: return "default_clause"sv;
        case kind::generation_clause: return "generation_clause"sv;
        case kind::identity_column: return "identity_column"sv;
    }
    std::abort();
}

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
inline std::ostream& operator<<(std::ostream& out, constraint_kind value) {
    return out << to_string_view(value);
}

} // namespace mizugaki::ast::statement
