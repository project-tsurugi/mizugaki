#pragma once

#include <mizugaki/ast/node.h>
#include <takatori/util/object_creator.h>

#include "join_specification_kind.h"

namespace mizugaki::ast::table {

/**
 * @brief columns specifications of queries.
 * @note `7.7 <joined table>` - `<join specification>`
 */
class join_specification : public node {
public:
    /// @brief the node kind type.
    using node_kind_type = join_specification_kind;

    using node::node;
    [[nodiscard]] join_specification* clone(::takatori::util::object_creator creator) const& override = 0;
    [[nodiscard]] join_specification* clone(::takatori::util::object_creator creator) && override = 0;

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
    friend bool operator==(join_specification const& a, join_specification const& b) noexcept;
    
    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(join_specification const& a, join_specification const& b) noexcept;
    
protected:
    /**
     * @brief compares to the given value.
     * @param other the comparison target
     * @return true if this and the value are equivalent
     * @return false otherwise
     */
    [[nodiscard]] virtual bool equals(join_specification const& other) const noexcept = 0;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, join_specification const& value);

} // namespace mizugaki::ast::table
