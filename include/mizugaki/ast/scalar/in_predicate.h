#pragma once

#include <takatori/util/object_creator.h>

#include <mizugaki/ast/common/regioned.h>
#include <mizugaki/ast/common/vector.h>

#include "expression.h"

namespace mizugaki::ast::scalar {

/**
 * @brief processes a quantified comparison.
 * @details This only supports `in` operator with value list.
 *      Otherwise, `in` operator with table subquery is represented in binary_expresion with `in` or `not_in`.
 * @note `8.4 <in predicate>`
 */
class in_predicate final : public expression {

    using super = expression;

public:
    /// @brief existence of `NOT` type.
    using not_type = common::regioned<bool>;

    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::in_predicate;

    /**
     * @brief creates a new instance.
     * @param left the left term
     * @param right the right term as (row) expression list
     * @param is_not whether or not `NOT` is declared
     * @param region the node region
     */
    explicit in_predicate(
            operand_type left,
            common::vector<operand_type> right,
            not_type is_not = {},
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit in_predicate(in_predicate const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit in_predicate(in_predicate&& other, ::takatori::util::object_creator creator);

    [[nodiscard]] in_predicate* clone(::takatori::util::object_creator creator) const& override;
    [[nodiscard]] in_predicate* clone(::takatori::util::object_creator creator) && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the left term.
     * @return the left term
     */
    [[nodiscard]] operand_type& left() noexcept;

    /// @copydoc left()
    [[nodiscard]] operand_type const& left() const noexcept;

    /**
     * @brief returns the right term.
     * @return the right term
     */
    [[nodiscard]] common::vector<operand_type>& right() noexcept;

    /// @copydoc right()
    [[nodiscard]] common::vector<operand_type> const& right() const noexcept;

    /**
     * @brief returns whether or not this declared as `NOT`.
     * @return true if `NOT` is declared
     * @return false otherwise
     */
    [[nodiscard]] not_type& is_not() noexcept;

    /// @copydoc is_not()
    [[nodiscard]] not_type const& is_not() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(in_predicate const& a, in_predicate const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(in_predicate const& a, in_predicate const& b) noexcept;

protected:
    [[nodiscard]] bool equals(expression const& other) const noexcept override;

private:
    operand_type left_;
    common::vector<operand_type> right_;
    not_type is_not_;
};

} // namespace mizugaki::ast::scalar
