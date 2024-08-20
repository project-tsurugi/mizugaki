#pragma once

#include <array>
#include <ostream>
#include <string>
#include <string_view>
#include <type_traits>

namespace mizugaki::parser {

/**
 * @brief represents syntactic element kind of SQL.
 */
enum class sql_parser_element_kind : std::size_t {
    /// @brief statements in compilation units.
    statement = 0,

    /// @brief elements in schema declarations.
    schema_element_definition,

    /// @brief elements in table declarations.
    table_element_definition,

    /// @brief constraint definitions in column declarations.
    column_constraint_definition,

    /// @brief column references in foreign key columns, join correlations, etc.
    column_reference,

    /// @brief storage parameters in table or index definitions.
    storage_parameter,

    /// @brief set clauses in `UPDATE` statement.
    set_clause,

    /// @brief `WITH` elements in `SELECT` statement.
    with_element,

    /// @brief elements in `SELECT` statement.
    select_element,

    /// @brief table references in `FROM` clause.
    table_reference,

    /// @brief column references in `GROUP BY` clause.
    grouping_element,

    /// @brief column references in `ORDER BY` clause.
    ordering_element,

    /// @brief field definitions in row type.
    field_definition,

    /// @brief expressions in table value constructor (e.g., `VALUES ...`).
    row_expression,

    /// @brief elements in expression list.
    scalar_expression,

    /// @brief elements in `CASE` expression.
    when_clause,

    /// @brief elements in string literal concatenations.
    string_literal_concatenation,
};

/**
 * @brief represents an associated list of sql_parser_element_map.
 * @tparam V the value type
 */
template<class V>
class sql_parser_element_map {
public:
    /// @brief the key type.
    using key_type = sql_parser_element_kind;

    /// @brief the value type.
    using value_type = V;

    /// @brief the reference type.
    using reference = std::add_lvalue_reference_t<value_type>;

    /// @brief the const reference type.
    using const_reference = std::add_lvalue_reference_t<std::add_const_t<value_type>>;

    /**
     * @brief returns the value corresponding to the given kind.
     * @tparam K the target kind
     * @return the value
     */
    template<key_type K>
    [[nodiscard]] constexpr reference get() noexcept {
        return entries_[static_cast<std::size_t>(K) - offset]; // NOLINT(*-bounds-constant-array-index)
    }

    /// @copydoc get()
    template<key_type K>
    [[nodiscard]] constexpr const_reference get() const noexcept {
        return entries_[static_cast<std::size_t>(K) - offset]; // NOLINT(*-bounds-constant-array-index)
    }

    /**
     * @brief returns the value corresponding to the given kind.
     * @param kind the target kind
     * @return the value
     */
    [[nodiscard]] constexpr reference operator[](key_type kind) noexcept {
        return entries_[static_cast<std::size_t>(kind) - offset]; // NOLINT(*-bounds-constant-array-index)
    }

    /// @copydoc operator[]()
    [[nodiscard]] constexpr const_reference operator[](key_type kind) const noexcept {
        return entries_[static_cast<std::size_t>(kind) - offset]; // NOLINT(*-bounds-constant-array-index)
    }

private:
    static constexpr std::size_t offset = static_cast<std::size_t>(key_type::statement);

    static constexpr std::size_t capacity =
            static_cast<std::size_t>(key_type::string_literal_concatenation) + 1ULL - offset;

    std::array<V, capacity> entries_ {};
};

/**
 * @brief returns string representation of the value.
 * @param value the target value
 * @return the corresponded string representation
 */
inline constexpr std::string_view to_string_view(sql_parser_element_kind value) noexcept {
    using namespace std::string_view_literals;
    using kind = sql_parser_element_kind;
    switch (value) {
        case kind::statement: return "statement"sv;
        case kind::schema_element_definition: return "schema_element_definition"sv;
        case kind::table_element_definition: return "table_element_definition"sv;
        case kind::column_constraint_definition: return "column_constraint_definition"sv;
        case kind::column_reference: return "column_reference"sv;
        case kind::storage_parameter: return "storage_parameter"sv;
        case kind::set_clause: return "set_clause"sv;
        case kind::with_element: return "with_element"sv;
        case kind::select_element: return "select_element"sv;
        case kind::table_reference: return "table_reference"sv;
        case kind::grouping_element: return "grouping_element"sv;
        case kind::ordering_element: return "ordering_element"sv;
        case kind::field_definition: return "field_definition"sv;
        case kind::row_expression: return "row_expression"sv;
        case kind::scalar_expression: return "scalar_expression"sv;
        case kind::when_clause: return "when_clause"sv;
        case kind::string_literal_concatenation: return "string_literal_concatenation"sv;
    }
    std::abort();
}

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
inline std::ostream& operator<<(std::ostream& out, sql_parser_element_kind value) {
    return out << to_string_view(value);
}

} // namespace mizugaki::parser
