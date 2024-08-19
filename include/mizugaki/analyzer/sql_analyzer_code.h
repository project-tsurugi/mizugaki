#pragma once

#include <ostream>
#include <string>
#include <string_view>

namespace mizugaki::analyzer {

/**
 * @brief represents diagnostic code of SQL analyzer.
 */
enum class sql_analyzer_code {
    /// @brief unknown diagnostic.
    unknown = 0,

    /// @brief the target feature is not supported.
    unsupported_feature,
    /// @brief the target has wrong syntax.
    malformed_syntax,

    // about literals
    /// @brief `DEFAULT` is specified not in suitable context.
    missing_context_of_default_value,
    /// @brief `NULL` is specified not in suitable context.
    missing_context_of_null,
    /// @brief the decimal value is unsupported range.
    unsupported_decimal_value,
    /// @brief the approximate number is malformed.
    malformed_approximate_number,
    /// @brief the number is out of range.
    unsupported_approximate_number,
    /// @brief the quoted string is malformed.
    malformed_quoted_string,
    /// @brief the string value is too long.
    unsupported_string_value,

    // about types
    /// @brief the flexible length or precision is not supported in this type.
    flexible_length_is_not_supported,
    /// @brief the type length or precision is not valid.
    invalid_type_length,
    /// @brief the numeric scale is too large for the precision.
    invalid_numeric_scale,

    /// @brief there is no such the catalog in system.
    catalog_not_found,
    /// @brief there is no such the schema in catalog.
    schema_not_found,
    /// @brief there is no such the table.
    table_not_found,
    /// @brief there is no such the index.
    index_not_found,
    /// @brief there is no such the view.
    view_not_found,
    /// @brief there is no such the sequence.
    sequence_not_found,
    /// @brief there is no such the column in the relation.
    column_not_found,
    /// @brief there is no such the variable.
    variable_not_found,
    /// @brief there is no such the function.
    function_not_found,
    /// @brief there is no such the symbol.
    symbol_not_found,

    /// @brief such the schema is already present.
    schema_already_exists,
    /// @brief such the table is already present.
    table_already_exists,
    /// @brief such the index is already present.
    index_already_exists,
    /// @brief such the view is already present.
    view_already_exists,
    /// @brief such the sequence is already present.
    sequence_already_exists,
    /// @brief such the column is already present.
    column_already_exists,
    /// @brief such the variable is already present.
    variable_already_exists,
    /// @brief such the function is already present.
    function_already_exists,
    /// @brief such the symbol is already present.
    symbol_already_exists,

    /// @brief there is no primary index for the table.
    primary_index_not_found,
    /// @brief the primary index is already present.
    primary_index_already_exists,

    /// @brief the target constraint is invalid.
    invalid_constraint,

    /// @brief the value is not suitable for the sequence.
    invalid_sequence_value,

    /// @brief there are ambiguous variables.
    relation_ambiguous,
    /// @brief there are ambiguous variables.
    variable_ambiguous,
    /// @brief there are ambiguous columns.
    column_ambiguous,
    /// @brief there are ambiguous function overloads.
    function_ambiguous,

    /// @brief value must be a unsigned integer
    invalid_unsigned_integer,

    /// @brief mismatch table.
    inconsistent_table,
    /// @brief mismatch table columns.
    inconsistent_columns,
    /// @brief aggregation target column exists at wrong position.
    invalid_aggregation_column,

    /// @brief input type is not distinguished for the overloaded operations.
    ambiguous_type,
    /// @brief the set of type input is inconsistent for this operation.
    inconsistent_type,
    /// @brief the declared elements exceeds the limit of count.
    exceed_number_of_elements,
    /// @brief the referring variable is not resolved (internal error).
    unresolved_variable,
    /// @brief the number of values is wrong (internal error).
    inconsistent_elements,
};

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(sql_analyzer_code value) noexcept {
    using namespace std::string_view_literals;
    using kind = sql_analyzer_code;
    switch (value) {
        case kind::unknown: return "unknown"sv;
        case kind::unsupported_feature: return "unsupported_feature"sv;
        case kind::malformed_syntax: return "malformed_syntax"sv;

        case kind::missing_context_of_default_value: return "missing_context_of_default_value"sv;
        case kind::missing_context_of_null: return "missing_context_of_null"sv;
        case kind::unsupported_decimal_value: return "unsupported_decimal_value"sv;
        case kind::malformed_approximate_number: return "malformed_approximate_number"sv;
        case kind::unsupported_approximate_number: return "unsupported_approximate_number"sv;
        case kind::malformed_quoted_string: return "malformed_quoted_string"sv;
        case kind::unsupported_string_value: return "unsupported_string_value"sv;

        case kind::flexible_length_is_not_supported: return "flexible_length_is_not_supported"sv;
        case kind::invalid_type_length: return "invalid_type_length"sv;
        case kind::invalid_numeric_scale: return "invalid_numeric_scale"sv;

        case kind::catalog_not_found: return "catalog_not_found"sv;
        case kind::schema_not_found: return "schema_not_found"sv;
        case kind::table_not_found: return "table_not_found"sv;
        case kind::index_not_found: return "index_not_found"sv;
        case kind::view_not_found: return "view_not_found"sv;
        case kind::sequence_not_found: return "sequence_not_found"sv;
        case kind::column_not_found: return "column_not_found"sv;
        case kind::variable_not_found: return "variable_not_found"sv;
        case kind::function_not_found: return "function_not_found"sv;
        case kind::symbol_not_found: return "symbol_not_found"sv;

        case kind::schema_already_exists: return "schema_already_exists"sv;
        case kind::table_already_exists: return "table_already_exists"sv;
        case kind::index_already_exists: return "index_already_exists"sv;
        case kind::view_already_exists: return "view_already_exists"sv;
        case kind::sequence_already_exists: return "sequence_already_exists"sv;
        case kind::column_already_exists: return "column_already_exists"sv;
        case kind::variable_already_exists: return "variable_already_exists"sv;
        case kind::function_already_exists: return "function_already_exists"sv;
        case kind::symbol_already_exists: return "symbol_already_exists"sv;

        case kind::primary_index_not_found: return "primary_index_not_found"sv;
        case kind::primary_index_already_exists: return "primary_index_already_exists"sv;

        case kind::invalid_constraint: return "invalid_constraint"sv;

        case kind::invalid_sequence_value: return "invalid_sequence_value"sv;

        case kind::relation_ambiguous: return "relation_ambiguous"sv;
        case kind::variable_ambiguous: return "variable_ambiguous"sv;
        case kind::column_ambiguous: return "column_ambiguous"sv;
        case kind::function_ambiguous: return "function_ambiguous"sv;

        case kind::inconsistent_table: return "inconsistent_table"sv;
        case kind::inconsistent_columns: return "inconsistent_columns"sv;
        case kind::invalid_aggregation_column: return "invalid_aggregation_column"sv;

        case kind::invalid_unsigned_integer: return "invalid_unsigned_integer"sv;

        case kind::ambiguous_type: return "ambiguous_type"sv;
        case kind::inconsistent_type: return "inconsistent_type"sv;
        case kind::exceed_number_of_elements: return "exceed_number_of_elements"sv;
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
inline std::ostream& operator<<(std::ostream& out, sql_analyzer_code value) {
    return out << to_string_view(value);
}

} // namespace mizugaki::analyzer
