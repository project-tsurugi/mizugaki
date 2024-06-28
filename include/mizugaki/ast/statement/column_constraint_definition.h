#pragma once

#include <takatori/util/clone_tag.h>
#include <takatori/util/rvalue_ptr.h>

#include <mizugaki/ast/element.h>
#include <mizugaki/ast/common/clone_wrapper.h>

#include <mizugaki/ast/name/name.h>

#include "constraint.h"

namespace mizugaki::ast::statement {

/**
 * @brief column constraint definition.
 * @note `11.4 <column definition>` - `<column constraint definition>`
 */
class column_constraint_definition final : public element {
public:
    /**
     * @brief creates a new empty instance.
     * @note this is used in parser generator.
     */
    explicit column_constraint_definition() = default;

    /**
     * @brief creates a new instance.
     * @param name the constraint name, or empty if it is not defined
     * @param body the body of constraint definition
     * @param region the element region
     */
    column_constraint_definition(
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
    column_constraint_definition( // NOLINT
            constraint&& body,
            ::takatori::util::rvalue_ptr<name::name> name = {},
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit column_constraint_definition(::takatori::util::clone_tag_t, column_constraint_definition const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit column_constraint_definition(::takatori::util::clone_tag_t, column_constraint_definition&& other);

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
    friend bool operator==(column_constraint_definition const& a, column_constraint_definition const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(column_constraint_definition const& a, column_constraint_definition const& b) noexcept;

    /**
     * @brief dumps structure information of the given value into the target acceptor.
     * @param acceptor the target acceptor
     * @param value the target value
     * @return the output
     */
    friend ::takatori::serializer::object_acceptor& operator<<(
            ::takatori::serializer::object_acceptor& acceptor,
            column_constraint_definition const& value);

private:
    common::clone_wrapper<std::unique_ptr<name::name>> name_;
    common::clone_wrapper<std::unique_ptr<constraint>> body_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, column_constraint_definition const& value);

} // namespace mizugaki::ast::statement
