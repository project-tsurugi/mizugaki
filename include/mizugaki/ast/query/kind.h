#pragma once

#include <ostream>
#include <string>
#include <string_view>

#include <takatori/util/enum_set.h>
#include <takatori/util/meta_type.h>

namespace mizugaki::ast::query {

class query;
class table_reference;
class table_value_constructor;
class binary_expression;
class with_expression;

/**
 * @brief represents a kind of query expression.
 * @note `7.12 <query expression>`
 */
enum class kind {
    /**
     * @copydoc query
     * @see query
     */
    query,

    /**
     * @copydoc table_reference
     * @see table_reference
     */
    table_reference,

    /**
     * @copydoc table_value_constructor
     * @see table_value_constructor
     */
    table_value_constructor,

    /**
     * @copydoc binary_expression
     * @see binary_expression
     */
    binary_expression,

    /**
     * @copydoc with_expression
     * @see with_expression
     */
    with_expression,
};

/// @brief set of query expression kind.
using kind_set = ::takatori::util::enum_set<
        kind,
        kind::query,
        kind::with_expression>;

/**
 * @brief provides the implementation type of the query expression kind.
 * @tparam Kind the value kind
 */
template<kind Kind> struct type_of;

/// @copydoc type_of
template<kind Kind> using type_of_t = typename type_of<Kind>::type;

/// @brief provides implementation type of kind::query.
template<> struct type_of<kind::query> : ::takatori::util::meta_type<query> {};

/// @brief provides implementation type of kind::table_reference.
template<> struct type_of<kind::table_reference> : ::takatori::util::meta_type<table_reference> {};

/// @brief provides implementation type of kind::table_value_constructor.
template<> struct type_of<kind::table_value_constructor> : ::takatori::util::meta_type<table_value_constructor> {};

/// @brief provides implementation type of kind::binary_expression.
template<> struct type_of<kind::binary_expression> : ::takatori::util::meta_type<binary_expression> {};

/// @brief provides implementation type of kind::with_expression.
template<> struct type_of<kind::with_expression> : ::takatori::util::meta_type<with_expression> {};

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(kind value) noexcept {
    using namespace std::string_view_literals;
    switch (value) {
        case kind::query: return "query"sv;
        case kind::table_reference: return "table_reference"sv;
        case kind::table_value_constructor: return "table_value_constructor"sv;
        case kind::binary_expression: return "binary_expression"sv;
        case kind::with_expression: return "with_expression"sv;
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

} // namespace mizugaki::ast::literal
