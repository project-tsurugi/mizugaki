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
     * @brief the default value of maximum precision of decimal numbers.
     * @see max_decimal_precision()
     */
    static constexpr size_type default_max_decimal_precision = 34;

    /**
     * @brief the default value of default precision of decimals.
     * @see default_decimal_precision()
     */
    static constexpr std::optional<size_type> default_default_decimal_precision =
            default_max_decimal_precision;

    /**
     * @brief the default value of maximum length of character strings.
     * @see max_character_string_length()
     */
    static constexpr size_type default_max_character_string_length = 4'096;

    /**
     * @brief the default value of default length of character strings.
     * @see default_character_string_length()
     */
    static constexpr std::optional<size_type> default_default_character_string_length =
            default_max_character_string_length;

    /**
     * @brief the default value of maximum length of bit strings.
     * @see max_bit_string_length()
     */
    static constexpr size_type default_max_bit_string_length = 32'768;

    /**
     * @brief the default value of default length of bit strings.
     * @see default_bit_string_length()
     */
    static constexpr std::optional<size_type> default_default_bit_string_length =
            default_max_bit_string_length;

    /**
     * @brief the default value of maximum length of octet strings.
     * @see max_octet_string_length()
     */
    static constexpr size_type default_max_octet_string_length = 32'768;

    /**
     * @brief the default value of default length of octet strings.
     * @see default_octet_string_length()
     */
    static constexpr std::optional<size_type> default_default_octet_string_length =
            default_max_octet_string_length;

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
     * @brief the default value of maximum length of collections.
     * @see max_collection_length()
     */
    static constexpr size_type default_max_collection_length = 1'024;

    /**
     * @brief the default value of maximum number of elements in rows.
     * @see max_row_elements()
     */
    static constexpr size_type default_max_row_elements = 1'024;

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
            ::takatori::util::maybe_shared_ptr<::yugawara::schema::search_path const> schema_search_path) noexcept;

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
     * @brief returns the maximum character string length.
     * @return the max character string length
     * @see #default_max_character_string_length
     */
    [[nodiscard]] size_type& max_character_string_length() noexcept {
        return max_character_string_length_;
    }

    /// @copydoc max_character_string_length()
    [[nodiscard]] size_type const& max_character_string_length() const noexcept {
        return max_character_string_length_;
    }

    /**
     * @brief returns the default character string length.
     * @return the default character string length
     * @see #default_default_character_string_length
     */
    [[nodiscard]] std::optional<size_type>& default_character_string_length() noexcept {
        return default_character_string_length_;
    }

    /// @copydoc default_character_string_length()
    [[nodiscard]] std::optional<size_type> const& default_character_string_length() const noexcept {
        return default_character_string_length_;
    }

    /**
     * @brief returns the maximum bit string length.
     * @return the max bit string length
     * @see #default_max_bit_string_length
     */
    [[nodiscard]] size_type& max_bit_string_length() noexcept {
        return max_bit_string_length_;
    }

    /// @copydoc max_bit_string_length()
    [[nodiscard]] size_type const& max_bit_string_length() const noexcept {
        return max_bit_string_length_;
    }

    /**
     * @brief returns the default bit string length.
     * @return the default bit string length
     * @see #default_default_bit_string_length
     */
    [[nodiscard]] std::optional<size_type>& default_bit_string_length() noexcept {
        return default_bit_string_length_;
    }

    /// @copydoc default_bit_string_length()
    [[nodiscard]] std::optional<size_type> const& default_bit_string_length() const noexcept {
        return default_bit_string_length_;
    }
    
    /**
     * @brief returns the maximum octet string length.
     * @return the max octet string length
     * @see #default_max_octet_string_length
     */
    [[nodiscard]] size_type& max_octet_string_length() noexcept {
        return max_octet_string_length_;
    }

    /// @copydoc max_octet_string_length()
    [[nodiscard]] size_type const& max_octet_string_length() const noexcept {
        return max_octet_string_length_;
    }

    /**
     * @brief returns the default octet string length.
     * @return the default octet string length
     * @see #default_default_octet_string_length
     */
    [[nodiscard]] std::optional<size_type>& default_octet_string_length() noexcept {
        return default_octet_string_length_;
    }

    /// @copydoc default_octet_string_length()
    [[nodiscard]] std::optional<size_type> const& default_octet_string_length() const noexcept {
        return default_octet_string_length_;
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
     * @brief returns the maximum collection length.
     * @return the max collection length
     * @see #default_max_collection_length
     */
    [[nodiscard]] size_type& max_collection_length() noexcept {
        return max_collection_length_;
    }

    /// @copydoc max_collection_length()
    [[nodiscard]] size_type const& max_collection_length() const noexcept {
        return max_collection_length_;
    }

    /**
     * @brief returns the maximum number of elements in each row.
     * @return the number of row elements
     * @see #default_max_row_elements
     */
    [[nodiscard]] size_type& max_row_elements() noexcept {
        return max_row_elements_;
    }

    /// @copydoc max_row_elements()
    [[nodiscard]] size_type const& max_row_elements() const noexcept {
        return max_row_elements_;
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

    size_type max_decimal_precision_ { default_max_decimal_precision };
    std::optional<size_type> default_decimal_precision_ { default_default_decimal_precision };
    size_type max_character_string_length_ { default_max_character_string_length };
    std::optional<size_type> default_character_string_length_ { default_default_character_string_length };
    size_type max_bit_string_length_ { default_max_bit_string_length };
    std::optional<size_type> default_bit_string_length_ { default_default_bit_string_length };
    size_type max_octet_string_length_ { default_max_octet_string_length };
    std::optional<size_type> default_octet_string_length_ { default_default_octet_string_length };

    size_type max_binary_integer1_precision_ { default_max_binary_integer1_precision };
    size_type max_binary_integer2_precision_ { default_max_binary_integer2_precision };
    size_type max_binary_integer4_precision_ { default_max_binary_integer4_precision };
    size_type max_binary_integer8_precision_ { default_max_binary_integer8_precision };
    size_type max_binary_float4_precision_ { default_max_binary_float4_precision };
    size_type max_binary_float8_precision_ { default_max_binary_float8_precision };
    size_type max_collection_length_ { default_max_collection_length };
    size_type max_row_elements_ { default_max_row_elements };
    bool allow_context_independent_null_ { default_allow_context_independent_null };

    std::string_view advance_sequence_function_name_ { default_advance_sequence_function_name };
};

} // namespace mizugaki::analyzer
