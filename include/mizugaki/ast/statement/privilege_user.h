#pragma once

#include <memory>

#include <mizugaki/ast/element.h>
#include <mizugaki/ast/common/clone_wrapper.h>
#include <mizugaki/ast/name/simple.h>

namespace mizugaki::ast::statement {

/**
 * @brief represents a user of privileges.
 * @note `10.5 <privileges> - <grantee>`
 */
class privilege_user : public element {
public:
    /**
     * creates a new instance.
     * @param authorization_identifier the user identifier, or empty pointer if not specified
     * @param region the element region
     */
    explicit privilege_user(
            std::unique_ptr<name::simple> authorization_identifier,
            region_type region) noexcept;

    /**
     * creates a new instance.
     * @param region the element region
     */
    privilege_user( // NOLINT
            region_type region = {}) noexcept;

    /**
     * creates a new instance.
     * @param authorization_identifier the user identifier
     * @param region the element region
     */
    privilege_user( // NOLINT
            name::simple&& authorization_identifier,
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit privilege_user(::takatori::util::clone_tag_t, privilege_user const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit privilege_user(::takatori::util::clone_tag_t, privilege_user&& other);

    /**
     * @brief returns the user of this element.
     * @return the authorization identifier
     * @return empty pointer if the authorization identifier is not specified
     */
    [[nodiscard]] std::unique_ptr<name::simple>& authorization_identifier() noexcept;

    /// @copydoc authorization_identifier()
    [[nodiscard]] std::unique_ptr<name::simple> const& authorization_identifier() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(privilege_user const& a, privilege_user const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(privilege_user const& a, privilege_user const& b) noexcept;

    /**
     * @brief dumps structure information of the given value into the target acceptor.
     * @param acceptor the target acceptor
     * @param value the target value
     * @return the output
     */
    friend ::takatori::serializer::object_acceptor& operator<<(
            ::takatori::serializer::object_acceptor& acceptor,
            privilege_user const& value);

private:
    common::clone_wrapper<std::unique_ptr<name::simple>> authorization_identifier_ {};
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, privilege_user const& value);

} // namespace mizugaki::ast::statement
