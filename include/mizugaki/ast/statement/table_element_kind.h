#pragma once

#include <ostream>
#include <string>
#include <string_view>
#include <utility>

#include <takatori/util/enum_set.h>
#include <takatori/util/meta_type.h>

namespace mizugaki::ast::statement {

class column_definition;
class table_constraint_definition;

/**
 * @brief represents a kind of table element.
 * @note `7.11 11.3 <table definition>` - `<table element>`
 */
enum class table_element_kind {
    /**
     * @brief declares a column.
     * @see table_column_definition
     */
    column_definition,

    /**
     * @brief defines a constraint.
     * @see table_constraint_definition
     */
    constraint_definition,
};

/// @brief set of table element kind.
using table_element_kind_set = ::takatori::util::enum_set<
        table_element_kind,
        table_element_kind::column_definition,
        table_element_kind::constraint_definition>;

template<class K, K Kind> struct type_of;

template<auto Kind> using type_of_t = typename type_of<decltype(Kind), Kind>::type;

/// @brief provides implementation type of table_element_kind::column_definition.
template<> struct type_of<table_element_kind, table_element_kind::column_definition> : ::takatori::util::meta_type<column_definition> {};

/// @brief provides implementation type of table_element_kind::table_constraint_definition.
template<> struct type_of<table_element_kind, table_element_kind::constraint_definition> : ::takatori::util::meta_type<table_constraint_definition> {};

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(table_element_kind value) noexcept {
    using namespace std::string_view_literals;
    using kind = table_element_kind;
    switch (value) {
        case kind::column_definition: return "column_definition"sv;
        case kind::constraint_definition: return "constraint_definition"sv;
    }
    std::abort();
}

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
inline std::ostream& operator<<(std::ostream& out, table_element_kind value) {
    return out << to_string_view(value);
}

} // namespace mizugaki::ast::statement
