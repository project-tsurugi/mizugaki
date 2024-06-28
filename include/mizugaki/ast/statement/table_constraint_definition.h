#pragma once

#include <takatori/util/clone_tag.h>
#include <takatori/util/rvalue_ptr.h>

#include <mizugaki/ast/node.h>

#include <mizugaki/ast/name/simple.h>

#include "table_element.h"
#include "constraint.h"

namespace mizugaki::ast::statement {

/**
 * @brief table constraint definition.
 * @note `11.6 <table constraint definition>`
 */
class table_constraint_definition final : public table_element {

    using super = table_element;

public:
    /// @brief the kind of this element.
    static constexpr table_element_kind tag = table_element_kind::constraint_definition;

    /**
     * @brief creates a new instance.
     * @param name the constraint name, or empty if it is not defined
     * @param body the body of constraint definition
     * @param region the element region
     */
    explicit table_constraint_definition(
            std::unique_ptr<name::name> name,
            std::unique_ptr<constraint> body,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param body the body of constraint definition
     * @param name the constraint name, or empty if it is not defined
     * @param region the element region
     * @attention this will take copy of arguments
     */
    explicit table_constraint_definition(
            constraint&& body,
            ::takatori::util::rvalue_ptr<name::name> name = {},
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit table_constraint_definition(::takatori::util::clone_tag_t, table_constraint_definition const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit table_constraint_definition(::takatori::util::clone_tag_t, table_constraint_definition&& other);

    [[nodiscard]] table_constraint_definition* clone() const& override;
    [[nodiscard]] table_constraint_definition* clone() && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the constraint name.
     * @return the constraint name
     * @return empty if it is not defined
     */
    [[nodiscard]] std::unique_ptr<name::name>& name() noexcept;

    /// @copydoc name()
    [[nodiscard]] std::unique_ptr<name::name> const& name() const noexcept;

    /**
     * @brief returns the constraint to define.
     * @return the constraint information
     */
    [[nodiscard]]std::unique_ptr<constraint>& body() noexcept;

    /// @copydoc body()
    [[nodiscard]]std::unique_ptr<constraint> const& body() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(table_constraint_definition const& a, table_constraint_definition const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(table_constraint_definition const& a, table_constraint_definition const& b) noexcept;

protected:
    [[nodiscard]] bool equals(table_element const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    std::unique_ptr<name::name> name_;
    std::unique_ptr<constraint> body_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, table_constraint_definition const& value);

} // namespace mizugaki::ast::statement
