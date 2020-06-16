#pragma once

#include <mizugaki/ast/node.h>

#include "kind.h"

namespace mizugaki::ast::literal {

class simple;

/**
 * @brief an abstract super interface of literal models.
 */
class literal : public node {
public:
    /// @brief the node kind type.
    using node_kind_type = kind;

    using node::node;
    [[nodiscard]] literal* clone(::takatori::util::object_creator creator) const& override = 0;
    [[nodiscard]] literal* clone(::takatori::util::object_creator creator) && override = 0;

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
    friend bool operator==(literal const& a, literal const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(literal const& a, literal const& b) noexcept;

protected:
    /**
     * @brief compares to the given value.
     * @param other the comparison target
     * @return true if this and the value are equivalent
     * @return false otherwise
     */
    [[nodiscard]] virtual bool equals(literal const& other) const noexcept = 0;
};

} // namespace mizugaki::ast::literal
