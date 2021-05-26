#pragma once

#include <mizugaki/ast/node.h>

#include "grouping_element_kind.h"

namespace mizugaki::ast::query {

/**
 * @brief columns specifications of queries.
 * @note `7.9 <group by clause>` - `<grouping specification>`
 */
class grouping_element : public node {
public:
    /// @brief the node kind type.
    using node_kind_type = grouping_element_kind;

    using node::node;
    [[nodiscard]] grouping_element* clone() const& override = 0;
    [[nodiscard]] grouping_element* clone() && override = 0;

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
    friend bool operator==(grouping_element const& a, grouping_element const& b) noexcept;
    
    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(grouping_element const& a, grouping_element const& b) noexcept;
    
protected:
    /**
     * @brief compares to the given value.
     * @param other the comparison target
     * @return true if this and the value are equivalent
     * @return false otherwise
     */
    [[nodiscard]] virtual bool equals(grouping_element const& other) const noexcept = 0;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, grouping_element const& value);

} // namespace mizugaki::ast::query
