#pragma once

#include <chrono>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include <takatori/util/maybe_shared_ptr.h>

#include <yugawara/schema/catalog.h>
#include <yugawara/schema/search_path.h>

namespace mizugaki::analyzer {

/**
 * @brief options of sql_analyzer.
 */
class sql_analyzer_options {
public:
    /// @brief the size type.
    using size_type = std::size_t;

    /**
     * @brief the default value of whether or not regular identifiers are treated as lowercase characters.
     * @see lowercase_regular_identifiers()
     */
    static constexpr bool default_lowercase_regular_identifiers = true;

    /**
     * @brief the default value of maximum precision of decimal numbers.
     * @see max_decimal_precision()
     */
    static constexpr size_type default_max_decimal_precision = 38;

    /**
     * @brief the default value of default precision of decimals.
     * @see default_decimal_precision()
     */
    static constexpr std::optional<size_type> default_default_decimal_precision =
            default_max_decimal_precision;

    /**
     * @brief the default value of maximum precision of binary integers.
     * @see max_binary_integer1_precision()
     */
    static constexpr size_type default_max_binary_integer1_precision = 7;

    /**
     * @brief the default value of maximum precision of binary integers.
     * @see max_binary_integer2_precision()
     */
    static constexpr size_type default_max_binary_integer2_precision = 15;

    /**
     * @brief the default value of maximum precision of binary integers.
     * @see max_binary_integer4_precision()
     */
    static constexpr size_type default_max_binary_integer4_precision = 31;

    /**
     * @brief the default value of maximum precision of binary integers.
     * @see max_binary_integer8_precision()
     */
    static constexpr size_type default_max_binary_integer8_precision = 63;

    /**
     * @brief the default value of maximum precision of binary floating point numbers.
     * @see max_binary_float4_precision()
     */
    static constexpr size_type default_max_binary_float4_precision = 25;

    /**
     * @brief the default value of maximum precision of binary floating point numbers.
     * @see max_binary_float8_precision()
     */
    static constexpr size_type default_max_binary_float8_precision = 53;

    /**
     * @brief the default value of whether or not to prefer to use write statement for simple insert operations.
     * @see prefer_write_statement()
     */
    static constexpr bool default_prefer_write_statement = true;

    /**
     * @brief the default value of whether or not to consider small integer literals (~2^31-1) as smaller data types.
     * @see prefer_small_integer_literals()
     */
    static constexpr bool default_prefer_small_integer_literals = false;

    /**
     * @brief the default value of whether to match the data length of string types to literals, or use `VARCHAR(*)`.
     * @see prefer_small_integer_literals()
     */
    static constexpr bool default_prefer_small_character_literals = false;

    /**
     * @brief the default value of whether to match the data length of binary string types to literals,
     *      or use `VARBINARY(*)`.
     * @see prefer_small_integer_literals()
     */
    static constexpr bool default_prefer_small_binary_literals = false;

    /**
     * @brief the default value of whether to match the precision of decimal types to literals, or use `DECIMAL(*)`.
     * @see prefer_small_integer_literals()
     */
    static constexpr bool default_prefer_small_decimal_literals = false;

    /**
     * @brief the default value of whether or not each host parameter declaration must be starts with column.
     * @see host_parameter_declaration_starts_with_colon()
     */
    static constexpr bool default_host_parameter_declaration_starts_with_colon = false;

    /**
     * @brief the default value of whether or not context independent `NULL` is allowed.
     * @see allow_context_independent_null()
     */
    static constexpr bool default_allow_context_independent_null = false;

    /**
     * @brief the default value of whether or not to validate all scalar expressions.
     * @see validate_all_scalar_expressions()
     */
    static constexpr bool default_validate_scalar_expressions = false;

    /**
     * @brief the default value of whether or not to automatically cast literals if the context type is specified.
     * @see cast_literals_in_context()
     */
    static constexpr bool default_cast_literals_in_context = true;

    /**
     * @brief the default value of the default to enable to wrap the seuqnce values.
     * @see default_sequence_cycle()
     */
    static constexpr bool default_default_sequence_cycle = true;

    /**
     * @brief the default value of a function name to advance a sequence value.
     * @see advance_sequence_function_name()
     */
    static constexpr std::string_view default_advance_sequence_function_name { "nextval" };

    /**
     * @brief the zone offset unit type.
     */
    using zone_offset_type = std::chrono::duration<std::int32_t, std::ratio<60>>;

    /**
     * @brief the default value of system time zone offset.
     * @see system_zone_offset()
     */
    static constexpr zone_offset_type default_system_zone_offset {}; // NOLINT(*-statically-constructed-objects)

    /**
     * @brief creates a new instance.
     * @param catalog the schema catalog
     * @param schema_search_path the schema search path
     * @param default_schema the default schema
     */
    sql_analyzer_options(
            ::takatori::util::maybe_shared_ptr<::yugawara::schema::catalog const> catalog,
            ::takatori::util::maybe_shared_ptr<::yugawara::schema::search_path const> schema_search_path,
            ::takatori::util::maybe_shared_ptr<::yugawara::schema::declaration const> default_schema) noexcept;

    /**
     * @brief returns the schema catalog to analyze AST.
     * @return the schema catalog
     */
    [[nodiscard]] ::yugawara::schema::catalog const& catalog() const noexcept;

    /**
     * @brief returns the schema search path.
     * @return the schema search path
     */
    [[nodiscard]] ::yugawara::schema::search_path const& schema_search_path() const noexcept;

    /**
     * @brief returns the default schema where to store the definitions without explicit schema.
     * @return the default schema
     */
    [[nodiscard]] ::yugawara::schema::declaration const& default_schema() const noexcept;

    /// @copydoc default_schema()
    [[nodiscard]] ::takatori::util::maybe_shared_ptr<::yugawara::schema::declaration const>  default_schema_shared() const noexcept;

    /**
     * @brief returns whether or not regular identifiers are treated as lowercase characters.
     * @return true if th regular identifiers are treated as lowercase characters
     * @return false otherwise
     */
    [[nodiscard]] bool& lowercase_regular_identifiers() noexcept {
        return lowercase_regular_identifiers_;
    }

    /// @copydoc lowercase_regular_identifiers()
    [[nodiscard]] bool lowercase_regular_identifiers() const noexcept {
        return lowercase_regular_identifiers_;
    }

    /**
     * @brief returns the maximum decimal precision.
     * @return the max decimal precision
     * @see #default_max_decimal_precision
     */
    [[nodiscard]] size_type& max_decimal_precision() noexcept {
        return max_decimal_precision_;
    }

    /// @copydoc max_decimal_precision()
    [[nodiscard]] size_type const& max_decimal_precision() const noexcept {
        return max_decimal_precision_;
    }

    /**
     * @brief returns the default decimal precision.
     * @return the default decimal precision
     * @see #default_default_decimal_precision
     */
    [[nodiscard]] std::optional<size_type>& default_decimal_precision() noexcept {
        return default_decimal_precision_;
    }

    /// @copydoc default_decimal_precision()
    [[nodiscard]] std::optional<size_type> const& default_decimal_precision() const noexcept {
        return default_decimal_precision_;
    }

    /**
     * @brief returns the maximum binary integer precision.
     * @return the max binary integer precision
     * @see #default_max_binary_integer1_precision
     */
    [[nodiscard]] size_type& max_binary_integer1_precision() noexcept {
        return max_binary_integer1_precision_;
    }

    /// @copydoc max_binary_integer1_precision()
    [[nodiscard]] size_type const& max_binary_integer1_precision() const noexcept {
        return max_binary_integer1_precision_;
    }

    /**
     * @brief returns the maximum binary integer precision.
     * @return the max binary integer precision
     * @see #default_max_binary_integer2_precision
     */
    [[nodiscard]] size_type& max_binary_integer2_precision() noexcept {
        return max_binary_integer2_precision_;
    }

    /// @copydoc max_binary_integer2_precision()
    [[nodiscard]] size_type const& max_binary_integer2_precision() const noexcept {
        return max_binary_integer2_precision_;
    }

    /**
     * @brief returns the maximum binary integer precision.
     * @return the max binary integer precision
     * @see #default_max_binary_integer4_precision
     */
    [[nodiscard]] size_type& max_binary_integer4_precision() noexcept {
        return max_binary_integer4_precision_;
    }

    /// @copydoc max_binary_integer4_precision()
    [[nodiscard]] size_type const& max_binary_integer4_precision() const noexcept {
        return max_binary_integer4_precision_;
    }

    /**
     * @brief returns the maximum binary integer precision.
     * @return the max binary integer precision
     * @see #default_max_binary_integer8_precision
     */
    [[nodiscard]] size_type& max_binary_integer8_precision() noexcept {
        return max_binary_integer8_precision_;
    }

    /// @copydoc max_binary_integer8_precision()
    [[nodiscard]] size_type const& max_binary_integer8_precision() const noexcept {
        return max_binary_integer8_precision_;
    }

    /**
     * @brief returns the maximum binary float precision for 8-octets floating point numbers.
     * @return the max number of bits in 4-octet float mantissa
     * @see #default_max_binary_float4_precision
     */
    [[nodiscard]] size_type& max_binary_float4_precision() noexcept {
        return max_binary_float4_precision_;
    }

    /// @copydoc max_binary_float4_precision()
    [[nodiscard]] size_type const& max_binary_float4_precision() const noexcept {
        return max_binary_float4_precision_;
    }

    /**
     * @brief returns the maximum binary float precision for 8-octets floating point numbers.
     * @return the max number of bits in 8-octet float mantissa
     * @see #default_max_binary_float8_precision
     */
    [[nodiscard]] size_type& max_binary_float8_precision() noexcept {
        return max_binary_float8_precision_;
    }

    /// @copydoc max_binary_float8_precision()
    [[nodiscard]] size_type const& max_binary_float8_precision() const noexcept {
        return max_binary_float8_precision_;
    }

    /**
     * @brief returns whether or not context independent `NULL` values are allowed.
     * @return true if they are allowed
     * @return false otherwise
     */
    [[nodiscard]] bool& allow_context_independent_null() noexcept {
        return allow_context_independent_null_;
    }

    /// @copydoc allow_context_independent_null()
    [[nodiscard]] bool const& allow_context_independent_null() const noexcept {
        return allow_context_independent_null_;
    }

    /**
     * @brief returns whether or not to prefer to use write statement for simple insert operations.
     * @return true if prefer to use write statement
     * @return false otherwise
     */
    [[nodiscard]] bool& prefer_write_statement() noexcept {
        return prefer_write_statement_;
    }

    /// @copydoc prefer_write_statement()
    [[nodiscard]] bool const& prefer_write_statement() const noexcept {
        return prefer_write_statement_;
    }

    /**
     * @brief returns whether or not each host parameter declaration must be starts with column.
     * @details If this is true, analyzer users must declare host parameters with column prefix.
     * @return true if each host parameter declaration must be starts with column
     * @return false otherwise
     */
    [[nodiscard]] bool& host_parameter_declaration_starts_with_colon() noexcept {
        return host_parameter_declaration_starts_with_colon_;
    };

    /// @copydoc host_parameter_declaration_starts_with_colon()
    [[nodiscard]] bool const& host_parameter_declaration_starts_with_colon() const noexcept {
        return host_parameter_declaration_starts_with_colon_;
    }

    /**
     * @brief returns whether or not to consider small integer literals (~2^31-1) as smaller data types.
     * @return true if prefer to smaller data types
     * @return false otherwise
     */
    [[nodiscard]] bool& prefer_small_integer_literals() noexcept {
        return prefer_small_integer_literals_;
    }

    /// @copydoc prefer_small_integer_literals()
    [[nodiscard]] bool const& prefer_small_integer_literals() const noexcept {
        return prefer_small_integer_literals_;
    }

    /**
     * @brief returns whether or not to consider small character literals as smaller data types.
     * @return true if prefer to smaller data types
     * @return false otherwise
     */
    [[nodiscard]] bool& prefer_small_character_literals() noexcept {
        return prefer_small_character_literals_;
    }

    /// @copydoc prefer_small_character_literals()
    [[nodiscard]] bool const& prefer_small_character_literals() const noexcept {
        return prefer_small_character_literals_;
    }

    /**
     * @brief returns whether or not to consider small binary string literals as smaller data types.
     * @return true if prefer to smaller data types
     * @return false otherwise
     */
    [[nodiscard]] bool& prefer_small_binary_literals() noexcept {
        return prefer_small_binary_literals_;
    }

    /// @copydoc prefer_small_binary_literals()
    [[nodiscard]] bool const& prefer_small_binary_literals() const noexcept {
        return prefer_small_binary_literals_;
    }

    /**
     * @brief returns whether or not to consider small decimal literals as smaller data types.
     * @return true if prefer to smaller data types
     * @return false otherwise
     */
    [[nodiscard]] bool& prefer_small_decimal_literals() noexcept {
        return prefer_small_decimal_literals_;
    }

    /// @copydoc prefer_small_decimal_literals()
    [[nodiscard]] bool const& prefer_small_decimal_literals() const noexcept {
        return prefer_small_decimal_literals_;
    }

    /**
     * @brief returns whether or not to validate all scalar expressions.
     * @details If disabled, the analyzer only check minimal scalar expressions to avoid unnecessary overhead.
     * @return true if validate all scalar expressions
     * @return false otherwise
     */
    [[nodiscard]] bool& validate_scalar_expressions() noexcept {
        return validate_scalar_expressions_;
    }

    /// @copydoc validate_scalar_expressions()
    [[nodiscard]] bool const& validate_scalar_expressions() const noexcept {
        return validate_scalar_expressions_;
    }

    /**
     * @brief returns whether or not to automatically cast literals if the context type is specified.
     * @return true if inserting cast is enabled
     * @return false otherwise
     */
    [[nodiscard]] bool& cast_literals_in_context() noexcept {
        return cast_literals_in_context_;
    }

    /// @copydoc cast_literals_in_context()
    [[nodiscard]] bool const& cast_literals_in_context() const noexcept {
        return cast_literals_in_context_;
    }

    /**
     * @brief returns whether or not to wrap the sequence values.
     * @return true if the sequence values are wrapped
     * @return false otherwise
     */
    [[nodiscard]] bool& default_sequence_cycle() noexcept {
        return default_sequence_cycle_;
    }

    /// @copydoc default_sequence_cycle()
    [[nodiscard]] bool const& default_sequence_cycle() const noexcept {
        return default_sequence_cycle_;
    }

    /**
     * @brief returns the function name to advance a sequence value.
     * @details This function requires a symbol of the target sequence.
     * @return the function name to advance a sequence value
     * @attention If you want to modify this, please care to keep the string lifecycle.
     */
    [[nodiscard]] std::string_view& advance_sequence_function_name() noexcept {
        return advance_sequence_function_name_;
    }

    /// @copydoc advance_sequence_function_name()
    [[nodiscard]] std::string_view const& advance_sequence_function_name() const noexcept {
        return advance_sequence_function_name_;
    }

    /**
     * @brief returns the system time zone offset.
     * @return return the system time zone offset
     */
    [[nodiscard]] zone_offset_type& system_zone_offset() noexcept {
        return system_zone_offset_;
    }

    /// @copydoc system_zone_offset()
    [[nodiscard]] zone_offset_type const& system_zone_offset() const noexcept {
        return system_zone_offset_;
    }

private:
    ::takatori::util::maybe_shared_ptr<::yugawara::schema::catalog const> catalog_;
    ::takatori::util::maybe_shared_ptr<::yugawara::schema::search_path const> schema_search_path_;
    ::takatori::util::maybe_shared_ptr<::yugawara::schema::declaration const> default_schema_;

    bool lowercase_regular_identifiers_ { default_lowercase_regular_identifiers };
    size_type max_decimal_precision_ { default_max_decimal_precision };
    std::optional<size_type> default_decimal_precision_ { default_default_decimal_precision };

    size_type max_binary_integer1_precision_ { default_max_binary_integer1_precision };
    size_type max_binary_integer2_precision_ { default_max_binary_integer2_precision };
    size_type max_binary_integer4_precision_ { default_max_binary_integer4_precision };
    size_type max_binary_integer8_precision_ { default_max_binary_integer8_precision };
    size_type max_binary_float4_precision_ { default_max_binary_float4_precision };
    size_type max_binary_float8_precision_ { default_max_binary_float8_precision };
    bool prefer_write_statement_ { default_prefer_write_statement };
    bool prefer_small_integer_literals_ { default_prefer_small_integer_literals };
    bool prefer_small_character_literals_ { default_prefer_small_character_literals };
    bool prefer_small_binary_literals_ { default_prefer_small_binary_literals };
    bool prefer_small_decimal_literals_ { default_prefer_small_decimal_literals };
    bool host_parameter_declaration_starts_with_colon_ { default_host_parameter_declaration_starts_with_colon };
    bool allow_context_independent_null_ { default_allow_context_independent_null };
    bool validate_scalar_expressions_ { default_validate_scalar_expressions };
    bool cast_literals_in_context_ { default_cast_literals_in_context };
    bool default_sequence_cycle_ { default_default_sequence_cycle };

    std::string_view advance_sequence_function_name_ { default_advance_sequence_function_name };
    zone_offset_type system_zone_offset_ { default_system_zone_offset };
};

} // namespace mizugaki::analyzer
