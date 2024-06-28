#pragma once

#include <ostream>
#include <string>
#include <string_view>

#include <takatori/util/enum_set.h>
#include <takatori/util/meta_type.h>

namespace mizugaki::ast::literal {

class boolean;
class numeric;
class string;
class datetime;
class interval;

/**
 * @brief represents a kind of literal.
 * @note `6.3 <value specification> and <target specification>`
 * @note `6.4 <contextually typed value specification>`
 */
enum class kind {

    /**
     * @brief boolean literals.
     * @see boolean
     */
    boolean,

    /**
     * @brief exact numeric literals.
     * @see numeric
     */
    exact_numeric,

    /**
     * @brief approximate numeric literals.
     * @see numeric
     */
    approximate_numeric,

    /**
     * @brief character string literals.
     * @see string
     */
    character_string,

    /**
     * @brief character string literals.
     * @see string
     */
    bit_string,

    /**
     * @brief hex/binary string literals.
     * @see string
     */
    hex_string,

    /**
     * @brief date literals.
     * @see datetime
     */
    date,

    /**
     * @brief time literals.
     * @see datetime
     */
    time,

    /**
     * @brief timestamp literals.
     * @see datetime
     */
    timestamp,

    /**
     * @brief interval literals.
     * @see interval
     */
    interval,

    /**
     * @brief `NULL`
     * @see special
     */
    null,

    /**
     * @brief `ARRAY[]`
     * @see empty
     */
    empty,

    /**
     * @brief `DEFAULT`
     * @see special
     */
    default_,
};

template<kind>
class special;

/**
 * @brief represents a null value.
 * @note `<null specification>`
 */
using null = special<kind::null>;

/**
 * @brief represents an untyped empty array.
 * @note `<empty specification>`
 */
using empty = special<kind::empty>;

/**
 * @brief represents a default value.
 * @note `<default specification>`
 */
using default_ = special<kind::default_>;

/// @brief set of literal kind.
using kind_set = ::takatori::util::enum_set<
        kind,
        kind::boolean,
        kind::default_>;

/**
 * @brief provides the implementation type of the value kind.
 * @tparam K the kind type
 * @tparam Kind the node kind
 */
template<class K, K Kind> struct type_of;

/**
 * @brief provides the implementation type of the node kind.
 * @tparam Kind the node kind
 */
template<kind Kind> using type_of_t = typename type_of<decltype(Kind), Kind>::type;

/// @brief provides implementation type of kind::boolean.
template<> struct type_of<kind, kind::boolean> : ::takatori::util::meta_type<boolean> {};

/// @brief provides implementation type of kind::exact_numeric.
template<> struct type_of<kind, kind::exact_numeric> : ::takatori::util::meta_type<numeric> {};

/// @brief provides implementation type of kind::approximate_numeric.
template<> struct type_of<kind, kind::approximate_numeric> : ::takatori::util::meta_type<numeric> {};

/// @brief provides implementation type of kind::character_string.
template<> struct type_of<kind, kind::character_string> : ::takatori::util::meta_type<string> {};

/// @brief provides implementation type of kind::bit_string.
template<> struct type_of<kind, kind::bit_string> : ::takatori::util::meta_type<string> {};

/// @brief provides implementation type of kind::hex_string.
template<> struct type_of<kind, kind::hex_string> : ::takatori::util::meta_type<string> {};

/// @brief provides implementation type of kind::date.
template<> struct type_of<kind, kind::date> : ::takatori::util::meta_type<datetime> {};

/// @brief provides implementation type of kind::time.
template<> struct type_of<kind, kind::time> : ::takatori::util::meta_type<datetime> {};

/// @brief provides implementation type of kind::timestamp.
template<> struct type_of<kind, kind::timestamp> : ::takatori::util::meta_type<datetime> {};

/// @brief provides implementation type of kind::interval.
template<> struct type_of<kind, kind::interval> : ::takatori::util::meta_type<interval> {};

/// @brief provides implementation type of kind::null.
template<> struct type_of<kind, kind::null> : ::takatori::util::meta_type<special<kind::null>> {};

/// @brief provides implementation type of kind::empty.
template<> struct type_of<kind, kind::empty> : ::takatori::util::meta_type<special<kind::empty>> {};

/// @brief provides implementation type of kind::default_.
template<> struct type_of<kind, kind::default_> : ::takatori::util::meta_type<special<kind::default_>> {};

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(kind value) noexcept {
    using namespace std::string_view_literals;
    switch (value) {
        case kind::boolean: return "boolean"sv;
        case kind::exact_numeric: return "exact_numeric"sv;
        case kind::approximate_numeric: return "approximate_numeric"sv;
        case kind::character_string: return "character_string"sv;
        case kind::bit_string: return "bit_string"sv;
        case kind::hex_string: return "hex_string"sv;
        case kind::date: return "date"sv;
        case kind::time: return "time"sv;
        case kind::timestamp: return "timestamp"sv;
        case kind::interval: return "interval"sv;
        case kind::null: return "null"sv;
        case kind::empty: return "empty"sv;
        case kind::default_: return "default"sv;
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
