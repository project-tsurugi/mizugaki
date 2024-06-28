#pragma once

#include <ostream>
#include <string>
#include <string_view>

#include <cstdlib>

#include <takatori/util/enum_set.h>

namespace mizugaki::analyzer {

/**
 * @brief represents operation result kind of sql_analyzer.
 */
enum class sql_analyzer_result_kind : std::size_t {
    /// @brief the result is not valid.
    diagnostics = 0,

    /// @brief intermediate execution plan.
    execution_plan,

    /// @brief statement except `execute`.
    statement,
};

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
constexpr std::string_view to_string_view(sql_analyzer_result_kind value) noexcept {
    using namespace std::string_view_literals;
    using kind = sql_analyzer_result_kind;
    switch (value) {
        case kind::diagnostics: return "diagnostics"sv;
        case kind::execution_plan: return "execution_plan"sv;
        case kind::statement: return "statement"sv;
    }
    std::abort();
}

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
inline std::ostream& operator<<(std::ostream& out, sql_analyzer_result_kind value) noexcept {
    return out << to_string_view(value);
}

} // namespace mizugaki::analyzer
