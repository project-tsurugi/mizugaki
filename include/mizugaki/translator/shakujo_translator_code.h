#pragma once

#include <ostream>
#include <string>
#include <string_view>

namespace mizugaki::translator {

/**
 * @brief represents diagnostic code of IR translator.
 */
enum class shakujo_translator_code {
    /// @brief unknown diagnostic.
    unknown = 0,
    /// @brief the input type is not supported.
    unsupported_type,
    /// @brief the input value is not supported.
    unsupported_value,
    /// @brief the input statement is not supported.
    unsupported_statement,
    /// @brief the input scalar expression is not supported.
    unsupported_scalar_expression,
    /// @brief the input relation expression is not supported.
    unsupported_relational_operator,

    /// @brief there is no such the table.
    table_not_found,
    /// @brief there is no such the column.
    column_not_found,
    /// @brief there is no such the variable.
    variable_not_found,
    /// @brief there is no such the function.
    function_not_found,

    /// @brief there are ambiguous function overloads.
    function_ambiguous,

    /// @brief mismatch table.
    inconsistent_table,
    /// @brief mismatch table columns.
    inconsistent_columns,
    /// @brief aggregation target column exists at wrong position.
    invalid_aggregation_column,

    /// @brief table is already defined.
    duplicate_table,
    /// @brief primary key is already defined.
    duplicate_primary_key,
    /// @brief column default value is not valid.
    invalid_default_value,

    /// @brief input type is not distinguished for the overloaded operations.
    ambiguous_type,
    /// @brief the set of type input is inconsistent for this operation.
    inconsistent_type,
    /// @brief the referring variable is not resolved.
    unresolved_variable,
    /// @brief the number of values is wrong.
    inconsistent_elements,
};

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(shakujo_translator_code value) noexcept {
    using namespace std::string_view_literals;
    using kind = shakujo_translator_code;
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
        case kind::duplicate_table: return "duplicate_table"sv;
        case kind::duplicate_primary_key: return "duplicate_primary_key"sv;
        case kind::invalid_default_value: return "invalid_default_value"sv;
        case kind::ambiguous_type: return "ambiguous_type"sv;
        case kind::inconsistent_type: return "inconsistent_type"sv;
        case kind::unresolved_variable: return "unresolved_variable"sv;
        case kind::inconsistent_elements: return "inconsistent_elements"sv;
    }
    std::abort();
}

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
inline std::ostream& operator<<(std::ostream& out, shakujo_translator_code value) {
    return out << to_string_view(value);
}

} // namespace mizugaki::translator
