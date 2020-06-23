#pragma once

#include <optional>

#include <takatori/util/object_creator.h>
#include <takatori/util/rvalue_ptr.h>

#include <mizugaki/ast/element.h>
#include <mizugaki/ast/common/clone_wrapper.h>
#include <mizugaki/ast/name/simple.h>
#include <mizugaki/ast/type/type.h>

namespace mizugaki::ast::type {

/**
 * @brief represents a definition of field of rows.
 * @note `6.2 <field definition>`
 */
class field_definition : public element {
public:
    /**
     * @brief creates a new empty instance.
     * @note this is used in parser generator.
     */
    explicit field_definition() = default;

    /**
     * @brief creates a new instance.
     * @param name the field name
     * @param type the field type
     * @param collation the optional collation name
     * @param region the element region
     */
    explicit field_definition(
            ::takatori::util::unique_object_ptr<name::simple> name,
            ::takatori::util::unique_object_ptr<ast::type::type> type,
            ::takatori::util::unique_object_ptr<name::name> collation = {},
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param name the field name
     * @param type the field type
     * @param collation the optional collation name
     * @param region the element region
     * @attention this will take copy of arguments
     */
    field_definition(
            name::simple&& name,
            ast::type::type&& type,
            ::takatori::util::rvalue_ptr<name::name> collation = {},
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit field_definition(field_definition const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit field_definition(field_definition&& other, ::takatori::util::object_creator creator);

    /**
     * @brief returns the field name.
     * @return the field name
     */
    [[nodiscard]] ::takatori::util::unique_object_ptr<name::simple>& name() noexcept;

    /// @copydoc name()
    [[nodiscard]] ::takatori::util::unique_object_ptr<name::simple> const& name() const noexcept;

    /**
     * @brief returns the field type.
     * @return the field type
     */
    [[nodiscard]] ::takatori::util::unique_object_ptr<ast::type::type>& type() noexcept;

    /// @copydoc type()
    [[nodiscard]] ::takatori::util::unique_object_ptr<ast::type::type> const& type() const noexcept;

    /**
     * @brief returns the collation name.
     * @return the collation name
     * @return empty if it is not declared
     */
    [[nodiscard]] ::takatori::util::unique_object_ptr<name::name>& collation() noexcept;

    /// @copydoc collation()
    [[nodiscard]] ::takatori::util::unique_object_ptr<name::name> const& collation() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(field_definition const& a, field_definition const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(field_definition const& a, field_definition const& b) noexcept;

    /**
     * @brief dumps structure information of the given value into the target acceptor.
     * @param acceptor the target acceptor
     * @param value the target value
     * @return the output
     */
    friend ::takatori::serializer::object_acceptor& operator<<(
            ::takatori::serializer::object_acceptor& acceptor,
            field_definition const& value);

private:
    common::clone_wrapper<::takatori::util::unique_object_ptr<name::simple>> name_;
    common::clone_wrapper<::takatori::util::unique_object_ptr<ast::type::type>> type_;
    common::clone_wrapper<::takatori::util::unique_object_ptr<name::name>> collation_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, field_definition const& value);

} // namespace mizugaki::ast::type
