#pragma once

#include <takatori/util/optional_ptr.h>

#include <mizugaki/ast/node.h>
#include <mizugaki/ast/common/chars.h>

#include "kind.h"

namespace mizugaki::ast::name {

class simple;

/**
 * @brief an abstract super interface of name models.
 */
class name : public node {
public:
    /// @brief the identifier type.
    using identifier_type = common::chars;

    /// @brief the node kind type.
    using node_kind_type = kind;

    using node::node;
    [[nodiscard]] name* clone() const& override = 0;
    [[nodiscard]] name* clone() && override = 0;

    /**
     * @brief returns the kind of this name.
     * @return the kind of this name
     */
    [[nodiscard]] virtual node_kind_type node_kind() const noexcept = 0;

    /**
     * @brief returns identifier of the last name.
     * @return the last identifier
     */
    [[nodiscard]] identifier_type const& last_identifier() const noexcept;

    /**
     * @brief returns the last name.
     * @return the last name
     */
    [[nodiscard]] virtual simple const& last_name() const noexcept = 0;

    /**
     * @brief returns the name qualifier.
     * @return the name qualifier
     * @return empty if this does not have any qualifier
     */
    [[nodiscard]] virtual ::takatori::util::optional_ptr<name const> optional_qualifier() const noexcept = 0;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(name const& a, name const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(name const& a, name const& b) noexcept;

protected:
    /**
     * @brief compares to the given value.
     * @param other the comparison target
     * @return true if this and the value are equivalent
     * @return false otherwise
     */
    [[nodiscard]] virtual bool equals(name const& other) const noexcept = 0;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, name const& value);

} // namespace mizugaki::ast::name
