#pragma once

#include <ostream>

#include <takatori/util/object_creator.h>
#include <takatori/serializer/object_acceptor.h>

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

    /**
     * @brief dumps structure information of the given value into the target acceptor.
     * @param acceptor the target acceptor
     * @param value the target value
     * @return the output
     */
    friend ::takatori::serializer::object_acceptor& operator<<(
            ::takatori::serializer::object_acceptor& acceptor,
            node const& value);

protected:
    /**
     * @brief serializes this object into the given acceptor.
     * @param acceptor the target acceptor
     */
    virtual void serialize(::takatori::serializer::object_acceptor& acceptor) const = 0;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, node const& value);

} // namespace mizugaki::ast
