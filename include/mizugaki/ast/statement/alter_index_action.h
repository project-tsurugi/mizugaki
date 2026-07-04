#pragma once

#include <mizugaki/ast/node.h>

#include "alter_index_action_kind.h"

namespace mizugaki::ast::statement {

/**
 * @brief action in alter index statement.
 * @note This is out of SQL standard
 * @see alter_index_statement
 */
class alter_index_action : public node {
public:
    /// @brief the node kind type.
    using node_kind_type = alter_index_action_kind;

    using node::node;
    [[nodiscard]] alter_index_action* clone() const& override = 0;
    [[nodiscard]] alter_index_action* clone() && override = 0;

    /**
     * @brief returns the kind of this action.
     * @return the kind of this action
     */
    [[nodiscard]] virtual node_kind_type node_kind() const noexcept = 0;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(alter_index_action const& a, alter_index_action const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(alter_index_action const& a, alter_index_action const& b) noexcept;

protected:
    /**
     * @brief compares to the given value.
     * @param other the comparison target
     * @return true if this and the value are equivalent
     * @return false otherwise
     */
    [[nodiscard]] virtual bool equals(alter_index_action const& other) const noexcept = 0;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, alter_index_action const& value);

} // namespace mizugaki::ast::statement
