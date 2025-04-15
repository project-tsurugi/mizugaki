#pragma once

#include <memory>
#include <optional>

#include <takatori/util/clone_tag.h>
#include <takatori/util/rvalue_ptr.h>

#include <mizugaki/ast/common/vector.h>
#include <mizugaki/ast/common/regioned.h>
#include <mizugaki/ast/common/rvalue_list.h>
#include <mizugaki/ast/name/name.h>

#include "statement.h"
#include "table_element.h"
#include "table_definition_option.h"
#include "storage_parameter.h"

namespace mizugaki::ast::statement {

/**
 * @brief defines a table.
 * @note `11.3 <table definition>`
 */
class table_definition final : public statement {

    using super = statement;

public:
    /// @brief the kind of this element.
    static constexpr node_kind_type tag = node_kind_type::table_definition;

    /// @brief option type with element region information.
    using option_type = common::regioned<table_definition_option>;

    /**
     * @brief creates a new instance.
     * @param name the table name
     * @param elements the table elements
     * @param options the declared options
     * @param parameters the storage parameters
     * @param description the region of description comment
     * @param region the node region
     */
    explicit table_definition(
            std::unique_ptr<name::name> name,
            std::vector<std::unique_ptr<table_element>> elements,
            std::vector<option_type> options = {},
            std::vector<storage_parameter> parameters = {},
            region_type description = {},
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param name the table name
     * @param elements the table elements
     * @param options the declared options
     * @param parameters the storage parameters
     * @param description the region of description comment
     * @param region the node region
     * @attention this will take copy of arguments
     */
    explicit table_definition(
            name::name&& name,
            common::rvalue_list<table_element> elements,
            std::initializer_list<option_type> options = {},
            std::initializer_list<storage_parameter> parameters = {},
            region_type description = {},
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit table_definition(::takatori::util::clone_tag_t, table_definition const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit table_definition(::takatori::util::clone_tag_t, table_definition&& other);

    [[nodiscard]] table_definition* clone() const& override;
    [[nodiscard]] table_definition* clone() && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the table name.
     * @return the table name
     */
    [[nodiscard]] std::unique_ptr<name::name>& name() noexcept;

    /// @copydoc name()
    [[nodiscard]] std::unique_ptr<name::name> const& name() const noexcept;

    /**
     * @brief returns the table elements.
     * @return the table elements
     */
    [[nodiscard]] std::vector<std::unique_ptr<table_element>>& elements() noexcept;

    /// @copydoc elements()
    [[nodiscard]] std::vector<std::unique_ptr<table_element>> const& elements() const noexcept;

    /**
     * @brief returns list of declared options.
     * @return declared options
     */
    [[nodiscard]] std::vector<option_type>& options() noexcept;

    /// @copydoc options()
    [[nodiscard]] std::vector<option_type> const& options() const noexcept;

    /**
     * @brief returns the storage parameters.
     * @return the storage parameters
     */
    [[nodiscard]] std::vector<storage_parameter>& parameters() noexcept;

    /// @copydoc parameters()
    [[nodiscard]] std::vector<storage_parameter> const& parameters() const noexcept;

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
    friend bool operator==(table_definition const& a, table_definition const& b) noexcept;
    
    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(table_definition const& a, table_definition const& b) noexcept;

protected:
    [[nodiscard]] bool equals(statement const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    std::unique_ptr<name::name> name_ {};
    std::vector<std::unique_ptr<table_element>> elements_ {};
    std::vector<option_type> options_ {};
    std::vector<storage_parameter> parameters_ {};
    region_type description_ {};
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, table_definition const& value);

} // namespace mizugaki::ast::statement
