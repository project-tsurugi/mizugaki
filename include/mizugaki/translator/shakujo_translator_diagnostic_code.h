#pragma once

namespace mizugaki::translator {

/**
 * @brief represents diagnostic code of IR translator.
 */
enum class shakujo_translator_diagnostic_code {
    /// @brief unknown diagnostic.
    unknown = 0,
    unsupported_type,
    unsupported_value,
    unsupported_statement,
    unsupported_scalar_expression,
    unsupported_relational_operator,
    table_not_found,
    column_not_found,
    variable_not_found,
    function_not_found,
    function_ambiguous,
    inconsistent_table,
    inconsistent_columns,
    invalid_aggregation_column,
    type_error,
};

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(shakujo_translator_diagnostic_code value) noexcept {
    using namespace std::string_view_literals;
    using kind = shakujo_translator_diagnostic_code;
    switch (value) {
        case kind::unknown: return "unknown"sv;
        case kind::unsupported_type: return "unsupported_type"sv;
        case kind::unsupported_value: return "unsupported_value"sv;
        case kind::unsupported_statement: return "unsupported_statement"sv;
        case kind::unsupported_scalar_expression: return "unsupported_scalar_expression"sv;
        case kind::unsupported_relational_operator: return "unsupported_relational_operator"sv;
        case kind::table_not_found: return "table_not_found"sv;
        case kind::column_not_found: return "column_not_found"sv;
        case kind::variable_not_found: return "variable_not_found"sv;
        case kind::function_not_found: return "function_not_found"sv;
        case kind::function_ambiguous: return "function_ambiguous"sv;
        case kind::inconsistent_table: return "inconsistent_table"sv;
        case kind::inconsistent_columns: return "inconsistent_columns"sv;
        case kind::invalid_aggregation_column: return "invalid_aggregation_column"sv;
        case kind::type_error: return "type_error"sv;
    }
    std::abort();
}

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
inline std::ostream& operator<<(std::ostream& out, shakujo_translator_diagnostic_code value) {
    return out << to_string_view(value);
}

} // namespace mizugaki::translator
