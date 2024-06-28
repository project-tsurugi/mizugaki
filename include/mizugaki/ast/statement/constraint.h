#pragma once

#include <mizugaki/ast/node.h>

#include "constraint_kind.h"

namespace mizugaki::ast::statement {

/**
 * @brief table or column constraints.
 * @note `11.4 <column definition>` - `<column constraint>`
 * @note `11.6 <table constraint definition>` - `<table constraint>`
 */
class constraint : public node {
public:
    /// @brief the node kind type.
    using node_kind_type = constraint_kind;

    using node::node;
    [[nodiscard]] constraint* clone() const& override = 0;
    [[nodiscard]] constraint* clone() && override = 0;

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
    friend bool operator==(constraint const& a, constraint const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(constraint const& a, constraint const& b) noexcept;

protected:
    /**
     * @brief compares to the given value.
     * @param other the comparison target
     * @return true if this and the value are equivalent
     * @return false otherwise
     */
    [[nodiscard]] virtual bool equals(constraint const& other) const noexcept = 0;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, constraint const& value);

} // namespace mizugaki::ast::statement
