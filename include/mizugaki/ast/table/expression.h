#pragma once

#include <mizugaki/ast/node.h>
#include <takatori/util/object_creator.h>

#include "kind.h"

namespace mizugaki::ast::table {

/**
 * @brief an abstract super interface of table expression models.
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
};

} // namespace mizugaki::ast::table
