#pragma once

#include <ostream>
#include <string>
#include <string_view>

#include <takatori/util/enum_set.h>
#include <takatori/util/meta_type.h>

namespace mizugaki::ast::scalar {

class literal_expression;
class variable_reference;
class field_reference;
class case_expression;
class cast_expression;
class unary_expression;
class binary_expression;
class value_constructor;
class subquery;
class comparison_predicate;
class between_predicate;
class in_predicate;
class pattern_match_predicate;
class function_invocation;
class set_function_invocation;
class builtin_function_invocation;
class builtin_set_function_invocation;
class new_invocation;
class method_invocation;
class static_method_invocation;

/**
 * @brief represents a kind of scalar expression.
 * @note `6 Scalar expressions`
 * @note `8 Predicates`
 */
enum class kind {
    /**
     * @copydoc literal_expression
     * @see literal_expression
     */
    literal_expression,

    /**
     * @copydoc variable_reference
     * @see variable_reference
     */
    variable_reference,

    /**
     * @copydoc field_reference
     * @see field_reference
     */
    field_reference,

    // FIXME: 6.13 <element reference>

    /**
     * @copydoc case_expression
     * @see case_expression
     */
    case_expression,

    /**
     * @copydoc cast_expression
     * @see cast_expression
     */
    cast_expression,

    /**
     * @copydoc unary_expression
     * @see unary_expression
     */
    unary_expression,

    /**
     * @copydoc binary_expression
     * @see binary_expression
     */
    binary_expression,

    // FIXME: <collate clause> (6.27 <string value expression>)
    // FIXME: <time zone clause> (6.28 <datetime value expression>)
    // FIXME: datetime_difference (6.29 <interval value expression>)
    // FIXME: 6.31 <array value expression>
    // FIXME: 6.32 <array value constructor>

    /**
     * @copydoc value_constructor
     * @see value_constructor
     */
    value_constructor,

    /**
     * @copydoc subquery
     * @see subquery
     */
    subquery,

    /**
     * @copydoc comparison_predicate
     * @see comparison_predicate
     */
    comparison_predicate,

    /**
     * @copydoc between_predicate
     * @see between_predicate
     */
    between_predicate,

    /**
     * @copydoc in_predicate
     * @see in_predicate
     */
    in_predicate,

    /**
     * @copydoc pattern_match_predicate
     * @see pattern_match_predicate
     */
    pattern_match_predicate,

    // FIXME: 8.11 <match predicate>
    // FIXME: 8.14 <type predicate>

    /**
     * @copydoc function_invocation
     * @see function_invocation
     */
    function_invocation,

    /**
     * @copydoc set_function_invocation
     * @see set_function_invocation
     */
    set_function_invocation,

    /**
     * @copydoc function_invocation
     * @see function_invocation
     */
    builtin_function_invocation,

    /**
     * @copydoc set_function_invocation
     * @see set_function_invocation
     */
    builtin_set_function_invocation,

    /**
     * @copydoc new_invocation
     * @see new_invocation
     */
    new_invocation,

    /**
     * @copydoc method_invocation
     * @see method_invocation
     */
    method_invocation,

    /**
     * @copydoc static_method_invocation
     * @see static_method_invocation
     */
    static_method_invocation,

    // FIXME: `CURRENT OF <cursor name>`
    // 14.6 <delete statement: positioned>
    // 14.9 <update statement: positioned>
};

/// @brief set of scalar expression kind.
using kind_set = ::takatori::util::enum_set<
        kind,
        kind::literal_expression,
        kind::static_method_invocation>;

/**
 * @brief provides the implementation type of the scalar expression kind.
 * @tparam K the kind type
 * @tparam Kind the node kind
 */
template<class K, K Kind> struct type_of;

/**
 * @brief provides the implementation type of the node kind.
 * @tparam Kind the node kind
 */
template<auto Kind> using type_of_t = typename type_of<decltype(Kind), Kind>::type;

/// @brief provides implementation type of kind::literal_expression.
template<> struct type_of<kind, kind::literal_expression> : ::takatori::util::meta_type<literal_expression> {};

/// @brief provides implementation type of kind::variable_reference.
template<> struct type_of<kind, kind::variable_reference> : ::takatori::util::meta_type<variable_reference> {};

/// @brief provides implementation type of kind::field_reference.
template<> struct type_of<kind, kind::field_reference> : ::takatori::util::meta_type<field_reference> {};

/// @brief provides implementation type of kind::subquery.
template<> struct type_of<kind, kind::subquery> : ::takatori::util::meta_type<subquery> {};

/// @brief provides implementation type of kind::case_expression.
template<> struct type_of<kind, kind::case_expression> : ::takatori::util::meta_type<case_expression> {};

/// @brief provides implementation type of kind::cast_expression.
template<> struct type_of<kind, kind::cast_expression> : ::takatori::util::meta_type<cast_expression> {};

/// @brief provides implementation type of kind::unary_expression.
template<> struct type_of<kind, kind::unary_expression> : ::takatori::util::meta_type<unary_expression> {};

/// @brief provides implementation type of kind::binary_expression.
template<> struct type_of<kind, kind::binary_expression> : ::takatori::util::meta_type<binary_expression> {};

/// @brief provides implementation type of kind::value_constructor.
template<> struct type_of<kind, kind::value_constructor> : ::takatori::util::meta_type<value_constructor> {};

/// @brief provides implementation type of kind::comparison_predicate.
template<> struct type_of<kind, kind::comparison_predicate> : ::takatori::util::meta_type<comparison_predicate> {};

/// @brief provides implementation type of kind::between_predicate.
template<> struct type_of<kind, kind::between_predicate> : ::takatori::util::meta_type<between_predicate> {};

/// @brief provides implementation type of kind::in_predicate.
template<> struct type_of<kind, kind::in_predicate> : ::takatori::util::meta_type<in_predicate> {};

/// @brief provides implementation type of kind::pattern_match_predicate.
template<> struct type_of<kind, kind::pattern_match_predicate> : ::takatori::util::meta_type<pattern_match_predicate> {};

/// @brief provides implementation type of kind::function_invocation.
template<> struct type_of<kind, kind::function_invocation> : ::takatori::util::meta_type<function_invocation> {};

/// @brief provides implementation type of kind::set_function_invocation.
template<> struct type_of<kind, kind::set_function_invocation> : ::takatori::util::meta_type<set_function_invocation> {};

/// @brief provides implementation type of kind::builtin_function_invocation.
template<> struct type_of<kind, kind::builtin_function_invocation> : ::takatori::util::meta_type<builtin_function_invocation> {};

/// @brief provides implementation type of kind::builtin_set_function_invocation.
template<> struct type_of<kind, kind::builtin_set_function_invocation> : ::takatori::util::meta_type<builtin_set_function_invocation> {};

/// @brief provides implementation type of kind::new_invocation.
template<> struct type_of<kind, kind::new_invocation> : ::takatori::util::meta_type<new_invocation> {};

/// @brief provides implementation type of kind::method_invocation.
template<> struct type_of<kind, kind::method_invocation> : ::takatori::util::meta_type<method_invocation> {};

/// @brief provides implementation type of kind::static_method_invocation.
template<> struct type_of<kind, kind::static_method_invocation> : ::takatori::util::meta_type<static_method_invocation> {};

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(kind value) noexcept {
    using namespace std::string_view_literals;
    switch (value) {
        case kind::literal_expression: return "literal_expression"sv;
        case kind::variable_reference: return "variable_reference"sv;
        case kind::field_reference: return "field_reference"sv;
        case kind::subquery: return "subquery"sv;
        case kind::case_expression: return "case_expression"sv;
        case kind::cast_expression: return "cast_expression"sv;
        case kind::unary_expression: return "unary_expression"sv;
        case kind::binary_expression: return "binary_expression"sv;
        case kind::value_constructor: return "value_constructor"sv;
        case kind::comparison_predicate: return "comparison_predicate"sv;
        case kind::between_predicate: return "between_predicate"sv;
        case kind::in_predicate: return "in_predicate"sv;
        case kind::pattern_match_predicate: return "pattern_match_predicate"sv;
        case kind::function_invocation: return "function_invocation"sv;
        case kind::set_function_invocation: return "set_function_invocation"sv;
        case kind::builtin_function_invocation: return "builtin_function_invocation"sv;
        case kind::builtin_set_function_invocation: return "builtin_set_function_invocation"sv;
        case kind::new_invocation: return "new_invocation"sv;
        case kind::method_invocation: return "method_invocation"sv;
        case kind::static_method_invocation: return "static_method_invocation"sv;
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

} // namespace mizugaki::ast::scalar
