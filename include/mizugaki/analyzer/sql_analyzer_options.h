#pragma once

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
     * @brief the default value of whether or not context independent `NULL` is allowed.
     * @see allow_context_independent_null()
     */
    static constexpr bool default_allow_context_independent_null = false;

    /**
     * @brief the default value of a function name to advance a sequence value.
     * @see advance_sequence_function_name()
     */
    static constexpr std::string_view default_advance_sequence_function_name { "nextval" };

    /**
     * @brief creates a new instance.
     * @param catalog the schema catalog
     * @param schema_search_path the schema search path
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
    bool allow_context_independent_null_ { default_allow_context_independent_null };

    std::string_view advance_sequence_function_name_ { default_advance_sequence_function_name };
};

} // namespace mizugaki::analyzer
