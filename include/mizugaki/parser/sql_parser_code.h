#pragma once

#include <ostream>
#include <string>
#include <string_view>

namespace mizugaki::parser {

/**
 * @brief represents diagnostic code of SQL analyzer.
 */
enum class sql_parser_code {
    /// @brief unknown diagnostic.
    unknown = 0,

    /// @brief diagnostics generated by parser system.
    system,

    /// @brief violates syntax rule restrictions.
    syntax_error,

    /// @brief the target feature is not supported.
    unsupported_feature,

    /// @brief the document includes unrecognized characters.
    invalid_character,

    /// @brief the token format is not appropriate for syntax.
    invalid_token,

    /// @brief the occurred token is not appropriate for syntax rules.
    unexpected_token,

    /// @brief the document includes unexpected EOF.
    unexpected_eof,
};

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(sql_parser_code value) noexcept {
    using namespace std::string_view_literals;
    using kind = sql_parser_code;
    switch (value) {
        case kind::unknown: return "unknown"sv;
        case kind::system: return "system"sv;
        case kind::syntax_error: return "syntax_error"sv;
        case kind::unsupported_feature: return "unsupported_feature"sv;
        case kind::invalid_character: return "invalid_character"sv;
        case kind::invalid_token: return "invalid_token"sv;
        case kind::unexpected_token: return "unexpected_token"sv;
        case kind::unexpected_eof: return "unexpected_eof"sv;
    }
    std::abort();
}

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
inline std::ostream& operator<<(std::ostream& out, sql_parser_code value) {
    return out << to_string_view(value);
}

} // namespace mizugaki::parser