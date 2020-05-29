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
};

} // namespace mizugaki::ast::query
