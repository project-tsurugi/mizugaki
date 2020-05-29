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
};

} // namespace mizugaki::ast::statement
