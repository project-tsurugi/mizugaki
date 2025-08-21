#pragma once

#include <takatori/util/clone_tag.h>

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
            std::unique_ptr<name::name> target,
            std::unique_ptr<scalar::expression> value,
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
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit set_element(::takatori::util::clone_tag_t, set_element const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit set_element(::takatori::util::clone_tag_t, set_element&& other);

    /**
     * @brief returns the update target.
     * @return the update target
     */
    [[nodiscard]] std::unique_ptr<name::name>& target() noexcept;

    /// @copydoc target()
    [[nodiscard]] std::unique_ptr<name::name> const& target() const noexcept;

    /**
     * @brief returns the value expression to set.
     * @return the update value
     */
    [[nodiscard]] std::unique_ptr<scalar::expression>& value() noexcept;

    /// @copydoc value()
    [[nodiscard]] std::unique_ptr<scalar::expression> const& value() const noexcept;

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

    /**
     * @brief dumps structure information of the given value into the target acceptor.
     * @param acceptor the target acceptor
     * @param value the target value
     * @return the output
     */
    friend ::takatori::serializer::object_acceptor& operator<<(
            ::takatori::serializer::object_acceptor& acceptor,
            set_element const& value);

private:
    common::clone_wrapper<std::unique_ptr<name::name>> target_;
    common::clone_wrapper<std::unique_ptr<scalar::expression>> value_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, set_element const& value);

} // namespace mizugaki::ast::statement
