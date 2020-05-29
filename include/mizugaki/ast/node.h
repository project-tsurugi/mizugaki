#pragma once

#include <takatori/util/object_creator.h>

#include "element.h"

namespace mizugaki::ast {

/**
 * @brief an abstract super interface of abstract syntax tree (AST) node.
 */
class node : public element {
public:
    /// @brief the region type of node.
    using region_type = node_region;

    /**
     * @brief creates a new instance.
     */
    constexpr node() = default;

    /**
     * @brief creates a new instance.
     * @param region the region of this node
     */
    explicit constexpr node(region_type region) noexcept :
        element { region }
    {}

    virtual ~node() = default;

    node(node const& other) noexcept = delete;
    node& operator=(node const& other) noexcept = delete;

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    node(node&& other) noexcept = default;

    /**
     * @brief assigns the given object.
     * @param other the move source
     * @return this
     */
    node& operator=(node&& other) noexcept = default;

    /**
     * @brief returns a clone of this node.
     * @param creator the object creator for building a copy
     * @return the created clone
     */
    [[nodiscard]] virtual node* clone(::takatori::util::object_creator creator) const& = 0;

    /// @copydoc clone()
    [[nodiscard]] virtual node* clone(::takatori::util::object_creator creator) && = 0;
};

} // namespace mizugaki::ast
