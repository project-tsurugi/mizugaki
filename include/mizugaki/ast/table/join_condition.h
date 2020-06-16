#pragma once

#include <takatori/util/object_creator.h>

#include <mizugaki/ast/element.h>
#include <mizugaki/ast/common/clone_wrapper.h>
#include <mizugaki/ast/name/name.h>
#include <mizugaki/ast/scalar/expression.h>

#include "join_specification.h"

namespace mizugaki::ast::table {

/**
 * @brief represents a join condition (`ON ...`).
 * @note `7.7 <joined table>` - `<join condition>`
 */
class join_condition : public join_specification {

    using super = join_specification;

public:
    /// @brief the kind of this element.
    static constexpr join_specification_kind tag = join_specification_kind::condition;

    /**
     * @brief creates a new instance.
     * @param expression the condition expression
     * @param region the element region
     */
    join_condition( // NOLINT: conversion constructor
            ::takatori::util::unique_object_ptr<scalar::expression> expression,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit join_condition(join_condition const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit join_condition(join_condition&& other, ::takatori::util::object_creator creator);

    [[nodiscard]] join_condition* clone(::takatori::util::object_creator creator) const& override;
    [[nodiscard]] join_condition* clone(::takatori::util::object_creator creator) && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the condition expression.
     * @return the condition expression
     */
    [[nodiscard]] ::takatori::util::unique_object_ptr<scalar::expression>& expression() noexcept;

    /// @copydoc expression()
    [[nodiscard]] ::takatori::util::unique_object_ptr<scalar::expression> const& expression() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(join_condition const& a, join_condition const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(join_condition const& a, join_condition const& b) noexcept;

protected:
    [[nodiscard]] bool equals(join_specification const& other) const noexcept override;

private:
    ::takatori::util::unique_object_ptr<scalar::expression> expression_;
};

} // namespace mizugaki::ast::table
