#pragma once

#include <string_view>

#include <cstddef>

#include <mizugaki/ast/scalar/extract_field_kind.h>

namespace mizugaki::analyzer::details {

/// @brief the built-in function name for `EXTRACT(YEAR FROM ...)`.
inline constexpr std::string_view extract_function_name_year { "extract_year" };

/// @brief the built-in function name for `EXTRACT(MONTH FROM ...)`.
inline constexpr std::string_view extract_function_name_month { "extract_month" };

/// @brief the built-in function name for `EXTRACT(DAY FROM ...)`.
inline constexpr std::string_view extract_function_name_day { "extract_day" };

/// @brief the built-in function name for `EXTRACT(HOUR FROM ...)`.
inline constexpr std::string_view extract_function_name_hour { "extract_hour" };

/// @brief the built-in function name for `EXTRACT(MINUTE FROM ...)`.
inline constexpr std::string_view extract_function_name_minute { "extract_minute" };

/// @brief the built-in function name for `EXTRACT(SECOND FROM ...)`.
inline constexpr std::string_view extract_function_name_second { "extract_second" };

/// @brief the built-in function name for `EXTRACT(TIMEZONE_HOUR FROM ...)`.
inline constexpr std::string_view extract_function_name_timezone_hour { "extract_timezone_hour" };

/// @brief the built-in function name for `EXTRACT(TIMEZONE_MINUTE FROM ...)`.
inline constexpr std::string_view extract_function_name_timezone_minute { "extract_timezone_minute" };

/// @brief the built-in function name for `EXTRACT(DATE FROM ...)`.
inline constexpr std::string_view extract_function_name_date { "date" };

/// @brief the built-in function name for `EXTRACT(YEAR TO MONTH FROM ...)`.
inline constexpr std::string_view extract_function_name_year_to_month { "extract_year_to_month" };

/// @brief the built-in function name for `EXTRACT(YEAR TO DAY FROM ...)`.
inline constexpr std::string_view extract_function_name_year_to_day { "extract_year_to_day" };

/// @brief the built-in function name for `EXTRACT(YEAR TO HOUR FROM ...)`.
inline constexpr std::string_view extract_function_name_year_to_hour { "extract_year_to_hour" };

/// @brief the built-in function name for `EXTRACT(YEAR TO MINUTE FROM ...)`.
inline constexpr std::string_view extract_function_name_year_to_minute { "extract_year_to_minute" };

/// @brief the built-in function name for `EXTRACT(YEAR TO SECOND FROM ...)`.
inline constexpr std::string_view extract_function_name_year_to_second { "extract_year_to_second" };

/**
 * @brief the fields of `EXTRACT` whose built-in function takes the second precision as the second argument.
 * @details The first argument is the operand of `EXTRACT`, and the second argument is the second precision
 *      (an `INT` value, from `0` to extract_max_second_precision).
 *      The built-in functions of the other fields only take the operand.
 */
inline constexpr ast::scalar::extract_field_kind_set extract_includes_second {
        ast::scalar::extract_field_kind::second,
        ast::scalar::extract_field_kind::year_to_second,
};

/**
 * @brief the max second precision of `EXTRACT`.
 * @details The second precision is this value if it is omitted, or it is capped to this value.
 * @see extract_includes_second
 */
inline constexpr std::size_t extract_max_second_precision { 9 };

/**
 * @brief returns the name of the built-in function which computes `EXTRACT` of the given field.
 * @param field the field kind
 * @return the corresponding built-in function name
 * @return empty if there is no such the function
 */
[[nodiscard]] constexpr std::string_view extract_function_name(ast::scalar::extract_field_kind field) noexcept {
    using kind = ast::scalar::extract_field_kind;
    switch (field) {
        case kind::year: return extract_function_name_year;
        case kind::month: return extract_function_name_month;
        case kind::day: return extract_function_name_day;
        case kind::hour: return extract_function_name_hour;
        case kind::minute: return extract_function_name_minute;
        case kind::second: return extract_function_name_second;
        case kind::timezone_hour: return extract_function_name_timezone_hour;
        case kind::timezone_minute: return extract_function_name_timezone_minute;
        case kind::date: return extract_function_name_date;
        case kind::year_to_month: return extract_function_name_year_to_month;
        case kind::year_to_day: return extract_function_name_year_to_day;
        case kind::year_to_hour: return extract_function_name_year_to_hour;
        case kind::year_to_minute: return extract_function_name_year_to_minute;
        case kind::year_to_second: return extract_function_name_year_to_second;
    }
    return {};
}

} // namespace mizugaki::analyzer::details
