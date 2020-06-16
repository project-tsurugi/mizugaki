#pragma once

#include <mizugaki/ast/node.h>
#include <takatori/util/object_creator.h>

#include "select_element_kind.h"

namespace mizugaki::ast::query {

/**
 * @brief columns specifications of queries.
 * @note `7.11 <query specification>` - `<select list>`
 */
class select_element : public node {
public:
    /// @brief the node kind type.
    using node_kind_type = select_element_kind;

    using node::node;
    [[nodiscard]] select_element* clone(::takatori::util::object_creator creator) const& override = 0;
    [[nodiscard]] select_element* clone(::takatori::util::object_creator creator) && override = 0;

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
    friend bool operator==(select_element const& a, select_element const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(select_element const& a, select_element const& b) noexcept;

protected:
    /**
     * @brief compares to the given value.
     * @param other the comparison target
     * @return true if this and the value are equivalent
     * @return false otherwise
     */
    [[nodiscard]] virtual bool equals(select_element const& other) const noexcept = 0;
};

} // namespace mizugaki::ast::query
