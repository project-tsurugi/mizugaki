#pragma once

#include <string>

#include <takatori/util/object_creator.h>

#include <mizugaki/ast/compilation_unit.h>
#include <mizugaki/ast/node_region.h>

namespace mizugaki::parser {

/**
 * @brief parses SQL text.
 */
class sql_parser_result {
public:
    /// @brief the resulting parsed model type.
    using value_type = ::takatori::util::unique_object_ptr<ast::compilation_unit>;

    /// @brief the location type.
    using location_type = ast::node_region;

    /// @brief the diagnostic message type.
    using message_type = std::string;

    /// @brief the erroneous information type.
    using error_type = std::pair<location_type, message_type>;

    /**
     * @brief creates a new empty instance.
     */
    sql_parser_result() = default;

    /**
     * @brief creates a new instance which represents a valid result.
     * @param value the valid value
     */
    sql_parser_result(value_type value) noexcept; // NOLINT: implicit conversion

    /**
     * @brief creates a new instance which represents erroneous information.
     * @param error erroneous information
     */
    sql_parser_result(error_type error) noexcept; // NOLINT: implicit conversion

    /**
     * @brief creates a new instance which represents erroneous information.
     * @param location the error location
     * @param message the error message
     */
    sql_parser_result(location_type location, message_type message) noexcept;

    /**
     * @brief returns whether or not this is a valid result.
     * @return true if this is a valid result, and value() contains the corresponded model
     * @return false if this is not a valid results
     */
    [[nodiscard]] bool has_value() const noexcept;

    /// @copydoc has_value()
    [[nodiscard]] explicit operator bool() const noexcept;

    /**
     * @brief returns the value result.
     * @return the valid result if it exists
     * @return empty if error was occurred
     * @see has_value()
     */
    [[nodiscard]] value_type& value() noexcept;

    /// @copydoc value()
    [[nodiscard]] value_type const& value() const noexcept;

    /// @copydoc value()
    [[nodiscard]] value_type& operator*() noexcept;

    /// @copydoc value()
    [[nodiscard]] value_type const& operator*() const noexcept;

    /**
     * @brief returns whether or not this object holds an erroneous information.
     * @return true if this object holds an erroneous information
     * @return false otherwise
     */
    [[nodiscard]] bool has_error() const noexcept;

    /**
     * @brief returns the holding erroneous information.
     * @return the normal value
     * @see has_error()
     * @warning undefined behavior if this object does not hold erroneous information
     */
    [[nodiscard]] error_type& error() noexcept;

    /// @copydoc error()
    [[nodiscard]] error_type const& error() const noexcept;

private:
    value_type value_ {};
    error_type error_ {};
};

} // namespace mizugaki::parser
