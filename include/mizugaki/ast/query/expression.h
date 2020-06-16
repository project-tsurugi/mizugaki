#pragma once

#include <mizugaki/ast/node.h>
#include <takatori/util/object_creator.h>

#include "kind.h"

namespace mizugaki::ast::query {

/**
 * @brief an abstract super interface of query expression models.
 */
class expression : public node {
public:
    /// @brief the node kind type.
    using node_kind_type = kind;

    using node::node;
    [[nodiscard]] expression* clone(::takatori::util::object_creator creator) const& override = 0;
    [[nodiscard]] expression* clone(::takatori::util::object_creator creator) && override = 0;

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
    friend bool operator==(expression const& a, expression const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(expression const& a, expression const& b) noexcept;

protected:
    /**
     * @brief compares to the given value.
     * @param other the comparison target
     * @return true if this and the value are equivalent
     * @return false otherwise
     */
    [[nodiscard]] virtual bool equals(expression const& other) const noexcept = 0;
};

} // namespace mizugaki::ast::query
