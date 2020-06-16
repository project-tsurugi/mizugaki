#pragma once

#include <mizugaki/ast/node.h>
#include <takatori/util/object_creator.h>

#include "kind.h"

namespace mizugaki::ast::statement {

/**
 * @brief an abstract super interface of SQL statement models.
 * @see `13.5 <SQL procedure statement>`
 */
class statement : public node {
public:
    /// @brief the node kind type.
    using node_kind_type = kind;

    using node::node;
    [[nodiscard]] statement* clone(::takatori::util::object_creator creator) const& override = 0;
    [[nodiscard]] statement* clone(::takatori::util::object_creator creator) && override = 0;

    /**
     * @brief returns the kind of this name.
     * @return the kind of this name
     */
    [[nodiscard]] virtual node_kind_type node_kind() const noexcept = 0;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(statement const& a, statement const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(statement const& a, statement const& b) noexcept;

protected:
    /**
     * @brief compares to the given value.
     * @param other the comparison target
     * @return true if this and the value are equivalent
     * @return false otherwise
     */
    [[nodiscard]] virtual bool equals(statement const& other) const noexcept = 0;
};

} // namespace mizugaki::ast::statement
