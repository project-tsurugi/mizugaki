#pragma once

#include <ostream>
#include <string>
#include <string_view>

#include <takatori/util/enum_set.h>
#include <takatori/util/meta_type.h>

namespace mizugaki::ast::scalar {

/**
 * @brief represents a kind of built-in function.
 * @note `6.17 <numeric value function>`
 * @note `6.18 <string value function>`
 * @note `6.19 <datetime value function>`
 * @note `6.20 <interval value function>`
 * @note `6.21 <case expression>`
 */
enum class builtin_function_kind {
    /**
     * @brief `POSITION`
     * @note `6.17 <numeric value function>` - `<position expression>`
     * @note format: `(:0 IN :1)`
     */
    position,

    /**
     * @brief `LENGTH`,
     * @note this is not in ANSI SQL99
     * @note format: `(:0)`
     */
    length,

    /**
     * @brief `{CHAR,CHARACTER}_LENGTH`,
     * @note `6.17 <numeric value function>` - `<length expression>`
     * @note format: `(:0)`
     */
    character_length,

    /**
     * @brief `OCTET_LENGTH`,
     * @note `6.17 <numeric value function>` - `<length expression>`
     * @note format: `(:0)`
     */
    octet_length,

    /**
     * @brief `BIT_LENGTH`,
     * @note `6.17 <numeric value function>` - `<length expression>`
     * @note format: `(:0)`
     */
    bit_length,

    // NOTE: `EXTRACT` is not a built-in function form

    /**
     * @brief `CARDINALITY`,
     * @note `6.17 <numeric value function>` - `<cardinality expression>`
     * @note format: `(:0)`
     */
    cardinality,

    /**
     * @brief `ABS`,
     * @note `6.17 <numeric value function>` - `<absolute value expression>`
     * @note format: `(:0)`
     */
    abs,

    /**
     * @brief `CEIL`,
     * @note `6.17 <numeric value function>` - `<absolute value expression>`
     * @note format: `(:0)`
     */
    ceil,

    /**
     * @brief `FLOOR`,
     * @note `6.17 <numeric value function>` - `<absolute value expression>`
     * @note format: `(:0)`
     */
    floor,

    /**
     * @brief `MOD`
     * @note `6.17 <numeric value function>` - `<modulus expression>`
     * @note format: `(:0 IN :1)`
     */
    mod,

    /**
     * @brief `SUBSTRING`
     * @note `6.18 <string value function>` - `<character substring function>`
     * @note `6.18 <string value function>` - `<regular expression substring function>`
     * @note format: `(:0 FROM :1 [FOR :2])`
     */
    substring,

    /**
     * @brief `UPPER`
     * @note `6.18 <string value function>` - `<upper>`
     * @note format: `(:0)`
     */
    upper,

    /**
     * @brief `LOWER`
     * @note `6.18 <string value function>` - `<lower>`
     * @note format: `(:0)`
     */
    lower,

    /**
     * @brief `CONVERT`
     * @note `6.18 <string value function>` - `<form-of-use conversion>`
     * @note format: `(:0 USING :1)`, where `:1` must be a variable_reference
     */
    convert,

    /**
     * @brief `TRANSLATE`
     * @note `6.18 <string value function>` - `<character translation>`
     * @note format: `(:0 USING :1)`, where `:1` must be a variable_reference
     */
    translate,


    // NOTE: `TRIM` is not a built-in function form

    /**
     * @brief `OVERLAY`
     * @note `6.18 <string value function>` - `<character overlay function>`
     * @note format: `(:0 PLACING :1 FROM :2 [FOR :3])`
     */
    overlay,

    /**
     * @brief `CURRENT_DATE`
     * @note `6.19 <datetime value function>` - `<current date value function>`
     * @note format: empty
     */
    current_date,

    /**
     * @brief `CURRENT_TIME`
     * @note `6.19 <datetime value function>` - `<current time value function>`
     * @note format: `[(:0)]`, where `:0` must be a literal_expression with unsigned integer
     */
    current_time,

    /**
     * @brief `LOCALTIME`
     * @note `6.19 <datetime value function>` - `<current local time value function>`
     * @note format: `[(:0)]`, where `:0` must be a literal_expression with unsigned integer
     */
    localtime,

    /**
     * @brief `CURRENT_TIMESTAMP`
     * @note `6.19 <datetime value function>` - `<current timestamp value function>`
     * @note format: `[(:0)]`, where `:0` must be a literal_expression with unsigned integer
     */
    current_timestamp,

    /**
     * @brief `LOCALTIMESTAMP`
     * @note `6.19 <datetime value function>` - `<current local timestamp value function>`
     * @note format: `[(:0)]`, where `:0` must be a literal_expression with unsigned integer
     */
    localtimestamp,

    /**
     * @brief `NULLIF`
     * @note `6.21 <case expression>` - `<case abbreviation>`
     * @note format: `(:0, :1)`
     */
    nullif,

    /**
     * @brief `COALESCE`
     * @note `6.21 <case expression>` - `<case abbreviation>`
     * @note format: `(:0 [, :1 [, :2 [, ...]]]])`
     */
    coalesce,

    /**
     * @brief `NEXT VALUE FOR`
     * @note `[SQL-2003] 6.13 <next value expression>`
     * @note format: `:0`, where `:0` must be a sequence name
     */
    next_value_for,

    /**
     * @brief `USER`
     * @note This is pseudo-function only used in default clause of column definitions.
     * @note `11.5 <default clause>` - `<default option>`
     * @note format: empty
     */
    user,

    /**
     * @brief `CURRENT_USER`
     * @note This is pseudo-function only used in default clause of column definitions.
     * @note `11.5 <default clause>` - `<default option>`
     * @note format: empty
     */
    current_user,

    /**
     * @brief `CURRENT_ROLE`
     * @note This is pseudo-function only used in default clause of column definitions.
     * @note `11.5 <default clause>` - `<default option>`
     * @note format: empty
     */
    current_role,

    /**
     * @brief `SYSTEM_USER`
     * @note This is pseudo-function only used in default clause of column definitions.
     * @note `11.5 <default clause>` - `<default option>`
     * @note format: empty
     */
    system_user,

    /**
     * @brief `CURRENT_PATH`
     * @note This is pseudo-function only used in default clause of column definitions.
     * @note `11.5 <default clause>` - `<default option>`
     * @note format: empty
     */
    current_path,
};

/// @brief set of builtin_function_kind kind.
using builtin_function_kind_set = ::takatori::util::enum_set<
        builtin_function_kind,
        builtin_function_kind::position,
        builtin_function_kind::current_path>;

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(builtin_function_kind value) noexcept {
    using namespace std::string_view_literals;
    using kind = builtin_function_kind;
    switch (value) {
        case kind::position: return "position"sv;
        case kind::length: return "length"sv;
        case kind::character_length: return "character_length"sv;
        case kind::octet_length: return "octet_length"sv;
        case kind::bit_length: return "bit_length"sv;
        case kind::cardinality: return "cardinality"sv;
        case kind::abs: return "abs"sv;
        case kind::ceil: return "ceil"sv;
        case kind::floor: return "floor"sv;
        case kind::mod: return "mod"sv;
        case kind::substring: return "substring"sv;
        case kind::upper: return "upper"sv;
        case kind::lower: return "lower"sv;
        case kind::convert: return "convert"sv;
        case kind::translate: return "translate"sv;
        case kind::overlay: return "overlay"sv;
        case kind::current_date: return "current_date"sv;
        case kind::current_time: return "current_time"sv;
        case kind::localtime: return "localtime"sv;
        case kind::current_timestamp: return "current_timestamp"sv;
        case kind::localtimestamp: return "localtimestamp"sv;
        case kind::nullif: return "nullif"sv;
        case kind::coalesce: return "coalesce"sv;
        case kind::next_value_for: return "next_value_for"sv;
        case kind::user: return "user"sv;
        case kind::current_user: return "current_user"sv;
        case kind::current_role: return "current_role"sv;
        case kind::system_user: return "system_user"sv;
        case kind::current_path: return "current_path"sv;
    }
    std::abort();
}

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
inline std::ostream& operator<<(std::ostream& out, builtin_function_kind value) {
    return out << to_string_view(value);
}

} // namespace mizugaki::ast::scalar
