#pragma once

#include <mizugaki/ast/node.h>

#include "kind.h"

namespace mizugaki::ast::type {

class simple;

/**
 * @brief an abstract super interface of type models.
 */
class type : public node {
public:
    /// @brief the node kind type.
    using node_kind_type = kind;

    using node::node;
    [[nodiscard]] type* clone(::takatori::util::object_creator creator) const& override = 0;
    [[nodiscard]] type* clone(::takatori::util::object_creator creator) && override = 0;

    /**
     * @brief returns the kind of this name.
     * @return the kind of this name
     */
    [[nodiscard]] virtual node_kind_type node_kind() const noexcept = 0;
};

} // namespace mizugaki::ast::type
