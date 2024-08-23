#pragma once

#include <string>


#include <mizugaki/ast/compilation_unit.h>
#include <mizugaki/ast/node_region.h>

#include "sql_parser_diagnostic.h"

namespace mizugaki::parser {

/**
 * @brief parses SQL text.
 */
class sql_parser_result {
public:
    /// @brief the resulting parsed model type.
    using value_type = std::unique_ptr<ast::compilation_unit>;

    /// @brief the location type.
    using location_type = ast::node_region;

    /// @brief the diagnostic type.
    using diagnostic_type = sql_parser_diagnostic;

    /// @brief the diagnostic message type.
    using message_type = diagnostic_type::message_type;

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
     * @param diagnostic erroneous information
     */
    sql_parser_result(diagnostic_type diagnostic) noexcept; // NOLINT: implicit conversion

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
     * @return empty if diagnostic was occurred
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
     * @brief returns whether or not this object holds an diagnostic information.
     * @return true if this object holds an diagnostic information
     * @return false otherwise
     */
    [[nodiscard]] bool has_diagnostic() const noexcept;

    /**
     * @brief returns the holding diagnostic information.
     * @return the normal value
     * @see has_diagnostic()
     * @warning undefined behavior if this object does not hold erroneous information
     */
    [[nodiscard]] diagnostic_type& diagnostic() noexcept;

    /// @copydoc diagnostic()
    [[nodiscard]] diagnostic_type const& diagnostic() const noexcept;

    /**
     * @brief returns the number of nodes in the AST.
     * @return the number of nodes in the AST
     * @return 0 if the valid AST does not exist
     * @return 0 if the check mechanism is not enabled
     */
    [[nodiscard]] std::size_t& tree_node_count() noexcept;

    /// @copydoc tree_node_count()
    [[nodiscard]] std::size_t tree_node_count() const noexcept;

    /**
     * @brief returns the depth of the AST.
     * @return the depth of the AST
     * @return 0 if the valid AST does not exist
     * @return 0 if the check mechanism is not enabled
     */
    [[nodiscard]] std::size_t& max_tree_depth() noexcept;

    /// @copydoc max_tree_depth()
    [[nodiscard]] std::size_t max_tree_depth() const noexcept;

private:
    value_type value_ {};
    diagnostic_type diagnostic_ {};

    std::size_t tree_node_count_ {};
    std::size_t max_tree_depth_ {};
};

} // namespace mizugaki::parser
