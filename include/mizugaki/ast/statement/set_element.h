#pragma once

#include <optional>

#include <takatori/util/object_creator.h>

#include <mizugaki/ast/element.h>
#include <mizugaki/ast/common/clone_wrapper.h>
#include <mizugaki/ast/name/name.h>
#include <mizugaki/ast/scalar/expression.h>

namespace mizugaki::ast::statement {

/**
 * @brief specifies update target and its value.
 * @note `14.9 <update statement: positioned>` - `<set clause>`
 */
class set_element : public element {
public:
    /**
     * @brief creates a new empty instance.
     * @note this is used in parser generator.
     */
    explicit set_element() = default;

    // FIXME: SET ROW = ...
    // FIXME: array suffix
    /**
     * @brief creates a new instance.
     * @param target the update target
     * @param value the value expression to set
     * @param region the element region
     */
    explicit set_element(
            ::takatori::util::unique_object_ptr<name::name> target,
            ::takatori::util::unique_object_ptr<scalar::expression> value,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param target the update target
     * @param value the value expression to set
     * @param region the element region
     */
    set_element(
            name::name&& target,
            scalar::expression&& value,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit set_element(set_element const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit set_element(set_element&& other, ::takatori::util::object_creator creator);

    /**
     * @brief returns the update target.
     * @return the update target
     */
    [[nodiscard]] ::takatori::util::unique_object_ptr<name::name>& target() noexcept;

    /// @copydoc target()
    [[nodiscard]] ::takatori::util::unique_object_ptr<name::name> const& target() const noexcept;

    /**
     * @brief returns the value expression to set.
     * @return the update value
     */
    [[nodiscard]] ::takatori::util::unique_object_ptr<scalar::expression>& value() noexcept;

    /// @copydoc value()
    [[nodiscard]] ::takatori::util::unique_object_ptr<scalar::expression> const& value() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(set_element const& a, set_element const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(set_element const& a, set_element const& b) noexcept;

private:
    common::clone_wrapper<::takatori::util::unique_object_ptr<name::name>> target_;
    common::clone_wrapper<::takatori::util::unique_object_ptr<scalar::expression>> value_;
};

} // namespace mizugaki::ast::statement
