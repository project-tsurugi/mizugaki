#pragma once

#include <ostream>
#include <string>
#include <string_view>

#include <takatori/util/enum_set.h>
#include <takatori/util/meta_type.h>

namespace mizugaki::ast::scalar {

/**
 * @brief represents a kind of built-in function.
 * @note `6.17 <numeric value function>` - `<extract field>`
 */
enum class extract_field_kind {
    /// @brief `YEAR`
    year,
    /// @brief `MONTH`
    month,
    /// @brief `DAY`
    day,
    /// @brief `HOUR`
    hour,
    /// @brief `MINUTE`
    minute,
    /// @brief `SECOND`
    second,
    /// @brief `TIMEZONE_HOUR`
    timezone_hour,
    /// @brief `TIMEZONE_MINUTE`
    timezone_minute,
};

/// @brief set of extract_field_kind kind.
using extract_field_kind_set = ::takatori::util::enum_set<
        extract_field_kind,
        extract_field_kind::year,
        extract_field_kind::timezone_minute>;

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(extract_field_kind value) noexcept {
    using namespace std::string_view_literals;
    using kind = extract_field_kind;
    switch (value) {
        case kind::year: return "year"sv;
        case kind::month: return "month"sv;
        case kind::day: return "day"sv;
        case kind::hour: return "hour"sv;
        case kind::minute: return "minute"sv;
        case kind::second: return "second"sv;
        case kind::timezone_hour: return "timezone_hour"sv;
        case kind::timezone_minute: return "timezone_minute"sv;
    }
    std::abort();
}

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
inline std::ostream& operator<<(std::ostream& out, extract_field_kind value) {
    return out << to_string_view(value);
}

} // namespace mizugaki::ast::scalar
