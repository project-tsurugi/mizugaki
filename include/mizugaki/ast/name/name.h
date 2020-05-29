#pragma once

#include <takatori/util/optional_ptr.h>

#include <mizugaki/ast/node.h>

#include "kind.h"

namespace mizugaki::ast::name {

class simple;

/**
 * @brief an abstract super interface of name models.
 */
class name : public node {
public:
    /// @brief the node kind type.
    using node_kind_type = kind;

    using node::node;
    [[nodiscard]] name* clone(::takatori::util::object_creator creator) const& override = 0;
    [[nodiscard]] name* clone(::takatori::util::object_creator creator) && override = 0;

    /**
     * @brief returns the kind of this name.
     * @return the kind of this name
     */
    [[nodiscard]] virtual node_kind_type node_kind() const noexcept = 0;

    /**
     * @brief returns the last name.
     * @return the last name
     */
    [[nodiscard]] virtual simple& last() noexcept = 0;

    /// @copydoc last()
    [[nodiscard]] virtual simple const& last() const noexcept = 0;

    /**
     * @brief returns the name qualifier.
     * @return the name qualifier
     * @return empty if this does not have any qualifier
     */
    [[nodiscard]] virtual ::takatori::util::optional_ptr<name> optional_qualifier() noexcept = 0;

    /// @copydoc optional_qualifier()
    [[nodiscard]] virtual ::takatori::util::optional_ptr<name const> optional_qualifier() const noexcept = 0;
};

} // namespace mizugaki::ast::name
