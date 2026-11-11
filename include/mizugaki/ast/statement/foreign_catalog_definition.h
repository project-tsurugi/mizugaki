#pragma once

#include <memory>
#include <vector>

#include <takatori/util/clone_tag.h>
#include <takatori/util/rvalue_ptr.h>

#include <mizugaki/ast/common/regioned.h>
#include <mizugaki/ast/name/name.h>

#include "statement.h"
#include "foreign_catalog_definition_option.h"

namespace mizugaki::ast::statement {

/**
 * @brief defines a foreign catalog, which imports the catalog of a remote server.
 * @note `CREATE FOREIGN CATALOG [IF NOT EXISTS] <catalog name> SERVER [IF EXISTS] <server name>`
 */
class foreign_catalog_definition final : public statement {

    using super = statement;

public:
    /// @brief the kind of this element.
    static constexpr node_kind_type tag = node_kind_type::foreign_catalog_definition;

    /// @brief option type with element region information.
    using option_type = common::regioned<foreign_catalog_definition_option>;

    /**
     * @brief creates a new instance.
     * @param name the catalog name
     * @param server_name the server name
     * @param options the declared options
     * @param region the node region
     */
    explicit foreign_catalog_definition(
            std::unique_ptr<name::name> name,
            std::unique_ptr<name::name> server_name,
            std::vector<option_type> options = {},
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param name the catalog name
     * @param server_name the server name
     * @param options the declared options
     * @param region the node region
     */
    explicit foreign_catalog_definition(
            ::takatori::util::rvalue_ptr<name::name> name,
            ::takatori::util::rvalue_ptr<name::name> server_name,
            std::initializer_list<option_type> options = {},
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit foreign_catalog_definition(::takatori::util::clone_tag_t, foreign_catalog_definition const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit foreign_catalog_definition(::takatori::util::clone_tag_t, foreign_catalog_definition&& other);

    [[nodiscard]] foreign_catalog_definition* clone() const& override;
    [[nodiscard]] foreign_catalog_definition* clone() && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the catalog name.
     * @return the catalog name
     */
    [[nodiscard]] std::unique_ptr<name::name>& name() noexcept;

    /// @copydoc name()
    [[nodiscard]] std::unique_ptr<name::name> const& name() const noexcept;

    /**
     * @brief returns the server name.
     * @return the server name
     */
    [[nodiscard]] std::unique_ptr<name::name>& server_name() noexcept;

    /// @copydoc server_name()
    [[nodiscard]] std::unique_ptr<name::name> const& server_name() const noexcept;

    /**
     * @brief returns list of declared options.
     * @return declared options
     */
    [[nodiscard]] std::vector<option_type>& options() noexcept;

    /// @copydoc options()
    [[nodiscard]] std::vector<option_type> const& options() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(foreign_catalog_definition const& a, foreign_catalog_definition const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(foreign_catalog_definition const& a, foreign_catalog_definition const& b) noexcept;

protected:
    [[nodiscard]] bool equals(statement const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    std::unique_ptr<name::name> name_ {};
    std::unique_ptr<name::name> server_name_ {};
    std::vector<option_type> options_ {};
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, foreign_catalog_definition const& value);

} // namespace mizugaki::ast::statement
