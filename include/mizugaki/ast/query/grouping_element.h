#pragma once

#include <mizugaki/ast/node.h>
#include <takatori/util/object_creator.h>

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
    [[nodiscard]] grouping_element* clone(::takatori::util::object_creator creator) const& override = 0;
    [[nodiscard]] grouping_element* clone(::takatori::util::object_creator creator) && override = 0;

    /**
     * @brief returns the kind of this name.
     * @return the kind of this name
     */
    [[nodiscard]] virtual node_kind_type node_kind() const noexcept = 0;
};

} // namespace mizugaki::ast::query
