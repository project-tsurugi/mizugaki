#pragma once

#include <optional>

#include <takatori/util/clone_tag.h>
#include <takatori/util/rvalue_ptr.h>

#include <mizugaki/ast/element.h>
#include <mizugaki/ast/common/clone_wrapper.h>
#include <mizugaki/ast/name/name.h>
#include <mizugaki/ast/scalar/expression.h>

namespace mizugaki::ast::statement {

/**
 * @brief represents an optional storage parameter.
 */
class storage_parameter final : public element {
public:
    /**
     * @brief creates a new empty instance.
     * @note this is used in parser generator.
     */
    explicit storage_parameter() = default;

    /**
     * @brief creates a new instance.
     * @param name the parameter name
     * @param value the parameter value, or empty if omitted
     * @param region the element region
     */
    explicit storage_parameter(
            std::unique_ptr<name::name> name,
            std::unique_ptr<scalar::expression> value = {},
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param target the update target
     * @param value the value expression to set
     * @param region the element region
     */
    storage_parameter( // NOLINT
            name::name&& target,
            ::takatori::util::rvalue_ptr<scalar::expression> value = {},
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit storage_parameter(::takatori::util::clone_tag_t, storage_parameter const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit storage_parameter(::takatori::util::clone_tag_t, storage_parameter&& other);

    /**
     * @brief returns the parameter name.
     * @return the parameter name
     */
    [[nodiscard]] std::unique_ptr<name::name>& name() noexcept;

    /// @copydoc name()
    [[nodiscard]] std::unique_ptr<name::name> const& name() const noexcept;

    /**
     * @brief returns the parameter value.
     * @return the parameter value
     * @return empty if it is not defined
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
    friend bool operator==(storage_parameter const& a, storage_parameter const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(storage_parameter const& a, storage_parameter const& b) noexcept;

    /**
     * @brief dumps structure information of the given value into the target acceptor.
     * @param acceptor the target acceptor
     * @param value the target value
     * @return the output
     */
    friend ::takatori::serializer::object_acceptor& operator<<(
            ::takatori::serializer::object_acceptor& acceptor,
            storage_parameter const& value);

private:
    common::clone_wrapper<std::unique_ptr<name::name>> name_;
    common::clone_wrapper<std::unique_ptr<scalar::expression>> value_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, storage_parameter const& value);

} // namespace mizugaki::ast::statement
