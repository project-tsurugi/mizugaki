#pragma once

#include <ostream>
#include <string>
#include <string_view>

#include <takatori/util/enum_set.h>
#include <takatori/util/meta_type.h>

namespace mizugaki::ast::type {

class simple;
class character_string;
class bit_string;
class octet_string;
class decimal;
class binary_numeric;
class datetime;
class interval;
class row;
class user_defined;
class collection;

/**
 * @brief represents a kind of type.
 */
enum class kind {
    /**
     * @brief unknown type.
     * @see simple
     */
    unknown,

    /**
     * @brief character string type.
     * @see character_string
     */
    character,

    /**
     * @brief varying character string type.
     * @see character_string
     */
    character_varying,

    /**
     * @brief character large object type.
     */
    character_large_object,

    /**
     * @brief bit string type.
     * @see bit_string
     */
    bit,

    /**
     * @brief varying bit string type.
     * @see bit_string
     */
    bit_varying,

    /**
     * @brief octet string type.
     * @see octet_string
     */
    octet,

    /**
     * @brief varying octet string type.
     * @see octet_string
     */
    octet_varying,

    /**
     * @brief binary large object type.
     */
    binary_large_object,

    /**
     * @brief exact numeric type.
     * @see decimal
     * @note this may be equivalent to decimal
     */
    numeric,

    /**
     * @brief exact numeric type.
     * @see decimal
     */
    decimal,

    /**
     * @brief tiny integer type.
     * @see simple
     */
    tiny_integer,

    /**
     * @brief small integer type.
     * @see simple
     */
    small_integer,

    /**
     * @brief integer type.
     * @see simple
     */
    integer,

    /**
     * @brief big integer type.
     * @see simple
     */
    big_integer,

    /**
     * @brief default precision floating point number type.
     * @see simple
     */
    float_,

    /**
     * @brief single precision floating point number type.
     * @see simple
     */
    real,

    /**
     * @brief double precision floating point number type.
     * @see simple
     */
    double_precision,

    /**
     * @brief binary integer type.
     * @see binary_numeric
     */
    binary_integer,

    /**
     * @brief binary floating point number type.
     * @see binary_numeric
     */
    binary_float,

    /**
     * @brief boolean types.
     * @see simple
     */
    boolean,

    /**
     * @brief date types.
     * @see simple
     */
    date,

    /**
     * @brief time types.
     * @see datetime
     */
    time,

    /**
     * @brief timestamp types.
     * @see datetime
     */
    timestamp,

    /**
     * @brief interval types.
     * @see interval
     */
    interval,

    /**
     * @brief row types.
     * @see row
     */
    row,

    /**
     * @brief user defined types.
     * @see user_defined
     */
    user_defined,

    /**
     * @brief collection types.
     * @see collection
     */
    collection,
};

/// @brief set of type kind.
using kind_set = ::takatori::util::enum_set<
        kind,
        kind::unknown,
        kind::collection>;

/**
 * @brief provides the implementation type of the type kind.
 * @tparam K the kind type
 * @tparam Kind the node kind
 */
template<class K, K Kind> struct type_of;

/**
 * @brief provides the implementation type of the node kind.
 * @tparam Kind the node kind
 */
template<auto Kind> using type_of_t = typename type_of<decltype(Kind), Kind>::type;

/// @brief provides implementation type of kind::unknown.
template<> struct type_of<kind, kind::unknown> : ::takatori::util::meta_type<simple> {};

/// @brief provides implementation type of kind::character.
template<> struct type_of<kind, kind::character> : ::takatori::util::meta_type<character_string> {};

/// @brief provides implementation type of kind::character_varying.
template<> struct type_of<kind, kind::character_varying> : ::takatori::util::meta_type<character_string> {};

/// @brief provides implementation type of kind::character_large_object.
template<> struct type_of<kind, kind::character_large_object> : ::takatori::util::meta_type<simple> {};

/// @brief provides implementation type of kind::bit.
template<> struct type_of<kind, kind::bit> : ::takatori::util::meta_type<bit_string> {};

/// @brief provides implementation type of kind::bit_varying.
template<> struct type_of<kind, kind::bit_varying> : ::takatori::util::meta_type<bit_string> {};

/// @brief provides implementation type of kind::octet.
template<> struct type_of<kind, kind::octet> : ::takatori::util::meta_type<octet_string> {};

/// @brief provides implementation type of kind::octet_varying.
template<> struct type_of<kind, kind::octet_varying> : ::takatori::util::meta_type<octet_string> {};

/// @brief provides implementation type of kind::binary_large_object.
template<> struct type_of<kind, kind::binary_large_object> : ::takatori::util::meta_type<simple> {};

/// @brief provides implementation type of kind::numeric.
template<> struct type_of<kind, kind::numeric> : ::takatori::util::meta_type<decimal> {};

/// @brief provides implementation type of kind::decimal.
template<> struct type_of<kind, kind::decimal> : ::takatori::util::meta_type<decimal> {};

/// @brief provides implementation type of kind::tiny_integer.
template<> struct type_of<kind, kind::tiny_integer> : ::takatori::util::meta_type<simple> {};

/// @brief provides implementation type of kind::small_integer.
template<> struct type_of<kind, kind::small_integer> : ::takatori::util::meta_type<simple> {};

/// @brief provides implementation type of kind::integer.
template<> struct type_of<kind, kind::integer> : ::takatori::util::meta_type<simple> {};

/// @brief provides implementation type of kind::big_integer.
template<> struct type_of<kind, kind::big_integer> : ::takatori::util::meta_type<simple> {};

/// @brief provides implementation type of kind::float_.
template<> struct type_of<kind, kind::float_> : ::takatori::util::meta_type<simple> {};

/// @brief provides implementation type of kind::real.
template<> struct type_of<kind, kind::real> : ::takatori::util::meta_type<simple> {};

/// @brief provides implementation type of kind::double_precision.
template<> struct type_of<kind, kind::double_precision> : ::takatori::util::meta_type<simple> {};

/// @brief provides implementation type of kind::binary_integer.
template<> struct type_of<kind, kind::binary_integer> : ::takatori::util::meta_type<binary_numeric> {};

/// @brief provides implementation type of kind::binary_float.
template<> struct type_of<kind, kind::binary_float> : ::takatori::util::meta_type<binary_numeric> {};

/// @brief provides implementation type of kind::boolean.
template<> struct type_of<kind, kind::boolean> : ::takatori::util::meta_type<simple> {};

/// @brief provides implementation type of kind::date.
template<> struct type_of<kind, kind::date> : ::takatori::util::meta_type<simple> {};

/// @brief provides implementation type of kind::time.
template<> struct type_of<kind, kind::time> : ::takatori::util::meta_type<datetime> {};

/// @brief provides implementation type of kind::timestamp.
template<> struct type_of<kind, kind::timestamp> : ::takatori::util::meta_type<datetime> {};

/// @brief provides implementation type of kind::interval.
template<> struct type_of<kind, kind::interval> : ::takatori::util::meta_type<interval> {};

/// @brief provides implementation type of kind::row.
template<> struct type_of<kind, kind::row> : ::takatori::util::meta_type<row> {};

/// @brief provides implementation type of kind::user_defined.
template<> struct type_of<kind, kind::user_defined> : ::takatori::util::meta_type<user_defined> {};

/// @brief provides implementation type of kind::collection.
template<> struct type_of<kind, kind::collection> : ::takatori::util::meta_type<collection> {};

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(kind value) noexcept {
    using namespace std::string_view_literals;
    switch (value) {
        case kind::unknown: return "unknown"sv;
        case kind::character: return "character"sv;
        case kind::character_varying: return "character_varying"sv;
        case kind::character_large_object: return "character_large_object"sv;
        case kind::bit: return "bit"sv;
        case kind::bit_varying: return "bit_varying"sv;
        case kind::octet: return "octet"sv;
        case kind::octet_varying: return "octet_varying"sv;
        case kind::binary_large_object: return "binary_large_object"sv;
        case kind::numeric: return "numeric"sv;
        case kind::decimal: return "decimal"sv;
        case kind::tiny_integer: return "tiny_integer"sv;
        case kind::small_integer: return "small_integer"sv;
        case kind::integer: return "integer"sv;
        case kind::big_integer: return "big_integer"sv;
        case kind::float_: return "float"sv;
        case kind::real: return "real"sv;
        case kind::double_precision: return "double_precision"sv;
        case kind::binary_integer: return "binary_integer"sv;
        case kind::binary_float: return "binary_float"sv;
        case kind::boolean: return "boolean"sv;
        case kind::date: return "date"sv;
        case kind::time: return "time"sv;
        case kind::timestamp: return "timestamp"sv;
        case kind::interval: return "interval"sv;
        case kind::row: return "row"sv;
        case kind::user_defined: return "user_defined"sv;
        case kind::collection: return "collection"sv;
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

} // namespace mizugaki::ast::type
