#pragma once

#include <memory>
#include <optional>

#include <mizugaki/ast/element.h>
#include <mizugaki/ast/common/clone_wrapper.h>
#include <mizugaki/ast/common/regioned.h>
#include <mizugaki/ast/name/name.h>

#include "privilege_object_kind.h"

namespace mizugaki::ast::statement {

/**
 * @brief represents an object of object privilege.
 * @note `10.5 <privileges> - <object name>`
 */
class privilege_object : public element {
public:
    /// @brief object kind type.
    using object_kind_type = common::regioned<privilege_object_kind>;

    /**
     * @brief creates new dummy instance.
     */
    explicit privilege_object() noexcept;

    /**
     * @brief creates a new instance.
     * @param object_kind the object kind of the object, or empty if the object kind is not specified
     * @param object_name the target object name
     * @param region the element region
     */
    explicit privilege_object(
            std::optional<object_kind_type> object_kind,
            std::unique_ptr<name::name> object_name,
            region_type region) noexcept;

    /**
     * creates a new instance without object kind.
     * @param object_name the table name
     * @param region the element region
     */
    privilege_object( // NOLINT
            name::name&& object_name,
            region_type region = {});

    /**
     * creates a new instance.
     * @param object_kind the object kind of the object
     * @param object_name the target object name
     * @param region the element region
     */
    privilege_object(
            object_kind_type object_kind,
            name::name&& object_name,
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit privilege_object(::takatori::util::clone_tag_t, privilege_object const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit privilege_object(::takatori::util::clone_tag_t, privilege_object&& other);

    /**
     * @brief returns the object kind of this element.
     * @return the object kind
     * @return empty if the object kind is not specified
     */
    [[nodiscard]] std::optional<object_kind_type>& object_kind() noexcept;

    /// @copydoc object_kind()
    [[nodiscard]] std::optional<object_kind_type> const& object_kind() const noexcept;

    /**
     * @brief returns the object name of this element.
     * @return the object name
     */
    [[nodiscard]] std::unique_ptr<name::name>& object_name() noexcept;

    /// @copydoc object_name()
    [[nodiscard]] std::unique_ptr<name::name> const& object_name() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(privilege_object const& a, privilege_object const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(privilege_object const& a, privilege_object const& b) noexcept;

    /**
     * @brief dumps structure information of the given value into the target acceptor.
     * @param acceptor the target acceptor
     * @param value the target value
     * @return the output
     */
    friend ::takatori::serializer::object_acceptor& operator<<(
            ::takatori::serializer::object_acceptor& acceptor,
            privilege_object const& value);

private:
    std::optional<object_kind_type> object_kind_;
    common::clone_wrapper<std::unique_ptr<name::name>> object_name_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, privilege_object const& value);

} // namespace mizugaki::ast::statement
