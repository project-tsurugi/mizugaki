#pragma once

#include <memory>
#include <optional>

#include <takatori/util/clone_tag.h>
#include <takatori/util/rvalue_ptr.h>

#include <mizugaki/ast/common/vector.h>
#include <mizugaki/ast/common/regioned.h>
#include <mizugaki/ast/common/rvalue_list.h>
#include <mizugaki/ast/name/simple.h>
#include <mizugaki/ast/scalar/expression.h>

#include "statement.h"
#include "schema_definition_option.h"

namespace mizugaki::ast::statement {

/**
 * @brief defines a schema.
 * @note `11.1 <schema definition>`
 */
class schema_definition final : public statement {

    using super = statement;

public:
    /// @brief the kind of this element.
    static constexpr node_kind_type tag = node_kind_type::schema_definition;

    /// @brief option type with element region information.
    using option_type = common::regioned<schema_definition_option>;

    /**
     * @brief creates a new instance.
     * @param name the schema name
     * @param user_name the schema user name expression, may be special keyword like `CURRENT_USER`
     * @param elements the schema elements
     * @param options the declared options
     * @param description the region of description comment
     * @param region the node region
     */
    explicit schema_definition(
            std::unique_ptr<name::name> name,
            std::unique_ptr<name::simple> user_name = {},
            std::vector<std::unique_ptr<statement>> elements = {},
            std::vector<option_type> options = {},
            region_type description = {},
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param name the schema name
     * @param user_name the schema user name expression, may be special keyword like `CURRENT_USER`
     * @param elements the schema elements
     * @param options the declared options
     * @param description the region of description comment
     * @param region the node region
     */
    explicit schema_definition(
            ::takatori::util::rvalue_ptr<name::name> name,
            ::takatori::util::rvalue_ptr<name::simple> user_name = {},
            common::rvalue_list<statement> elements = {},
            std::initializer_list<option_type> options = {},
            region_type description = {},
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit schema_definition(::takatori::util::clone_tag_t, schema_definition const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit schema_definition(::takatori::util::clone_tag_t, schema_definition&& other);

    [[nodiscard]] schema_definition* clone() const& override;
    [[nodiscard]] schema_definition* clone() && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the schema name.
     * @return the schema name
     */
    [[nodiscard]] std::unique_ptr<name::name>& name() noexcept;

    /// @copydoc name()
    [[nodiscard]] std::unique_ptr<name::name> const& name() const noexcept;

    /**
     * @brief the schema user name.
     * @return the schema user name
     */
    [[nodiscard]] std::unique_ptr<name::simple>& user_name() noexcept;

    /// @copydoc user_name()
    [[nodiscard]] std::unique_ptr<name::simple> const& user_name() const noexcept;

    /**
     * @brief returns the schema elements.
     * @return the schema elements
     */
    [[nodiscard]] std::vector<std::unique_ptr<statement>>& elements() noexcept;

    /// @copydoc elements()
    [[nodiscard]] std::vector<std::unique_ptr<statement>> const& elements() const noexcept;

    /**
     * @brief returns list of declared options.
     * @return declared options
     */
    [[nodiscard]] std::vector<option_type>& options() noexcept;

    /// @copydoc options()
    [[nodiscard]] std::vector<option_type> const& options() const noexcept;

    /**
     * @brief returns the region of description comment for this definition.
     * @return the description comment region
     * @return the empty region if not specified
     */
    [[nodiscard]] region_type& description() noexcept;

    /// @copydoc description()
    [[nodiscard]] region_type const& description() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(schema_definition const& a, schema_definition const& b) noexcept;
    
    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(schema_definition const& a, schema_definition const& b) noexcept;

protected:
    [[nodiscard]] bool equals(statement const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    std::unique_ptr<name::name> name_ {};
    std::unique_ptr<name::simple> user_name_ {};
    std::vector<std::unique_ptr<statement>> elements_ {};
    std::vector<option_type> options_ {};
    region_type description_ {};
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, schema_definition const& value);

} // namespace mizugaki::ast::statement
