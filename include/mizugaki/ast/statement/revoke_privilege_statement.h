#pragma once

#include <vector>

#include "statement.h"
#include "privilege_object.h"
#include "privilege_action.h"
#include "privilege_user.h"

namespace mizugaki::ast::statement {

/**
 * @brief revoke resource privileges from users.
 * @note `12.6 <revoke statement>`
 */
class revoke_privilege_statement : public statement {

    using super = statement;

public:
    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::revoke_privilege_statement;

    /**
     * creates a new instance.
     * @param actions the privilege actions
     * @param objects the privilege objects
     * @param users the privilege target users
     * @param region the node region
     */
    explicit revoke_privilege_statement(
            std::vector<privilege_action> actions,
            std::vector<privilege_object> objects,
            std::vector<privilege_user> users,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit revoke_privilege_statement(::takatori::util::clone_tag_t, revoke_privilege_statement const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit revoke_privilege_statement(::takatori::util::clone_tag_t, revoke_privilege_statement&& other);

    [[nodiscard]] revoke_privilege_statement* clone() const& override;
    [[nodiscard]] revoke_privilege_statement* clone() && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the privilege actions.
     * @return privilege actions
     */
    [[nodiscard]] std::vector<privilege_action>& actions() noexcept;

    /// @copydoc actions()
    [[nodiscard]] std::vector<privilege_action> const& actions() const noexcept;

    /**
     * @brief returns the privilege objects.
     * @return privilege objects
     */
    [[nodiscard]] std::vector<privilege_object>& objects() noexcept;

    /// @copydoc objects()
    [[nodiscard]] std::vector<privilege_object> const& objects() const noexcept;

    /**
     * @brief returns the privilege target users.
     * @return privilege target users
     */
    [[nodiscard]] std::vector<privilege_user>& users() noexcept;

    /// @copydoc users()
    [[nodiscard]] std::vector<privilege_user> const& users() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(revoke_privilege_statement const& a, revoke_privilege_statement const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(revoke_privilege_statement const& a, revoke_privilege_statement const& b) noexcept;

protected:
    [[nodiscard]] bool equals(statement const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    std::vector<privilege_action> actions_;
    std::vector<privilege_object> objects_;
    std::vector<privilege_user> users_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, revoke_privilege_statement const& value);

} // namespace mizugaki::ast::statement
