#pragma once

#include <mizugaki/ast/element.h>
#include <mizugaki/ast/common/regioned.h>
#include <mizugaki/ast/name/simple.h>

#include "privilege_action_kind.h"

namespace mizugaki::ast::statement {

/**
 * @brief represents an action of object privilege.
 * @note `10.5 <privileges> - <action>`
 */
class privilege_action : public element {
public:
    /// @brief action kind type.
    using action_kind_type = common::regioned<privilege_action_kind>;

    /**
     * @brief creates a new dummy instance.
     */
    explicit privilege_action() noexcept;

    /**
     * @brief creates a new instance.
     * @param action_kind the action kind of the object
     * @param region the element region
     */
    privilege_action( // NOLINT
        action_kind_type action_kind,
        region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit privilege_action(::takatori::util::clone_tag_t, privilege_action const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit privilege_action(::takatori::util::clone_tag_t, privilege_action&& other);

    /**
     * @brief returns the action kind of this element.
     * @return the action kind
     */
    [[nodiscard]] action_kind_type& action_kind() noexcept;

    /// @copydoc action_kind()
    [[nodiscard]] action_kind_type const& action_kind() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(privilege_action const& a, privilege_action const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(privilege_action const& a, privilege_action const& b) noexcept;

    /**
     * @brief dumps structure information of the given value into the target acceptor.
     * @param acceptor the target acceptor
     * @param value the target value
     * @return the output
     */
    friend ::takatori::serializer::object_acceptor& operator<<(
            ::takatori::serializer::object_acceptor& acceptor,
            privilege_action const& value);

private:
    action_kind_type action_kind_ {};
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, privilege_action const& value);

} // namespace mizugaki::ast::statement
