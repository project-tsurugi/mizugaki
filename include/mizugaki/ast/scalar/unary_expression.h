#pragma once

#include <takatori/util/object_creator.h>

#include <mizugaki/ast/common/regioned.h>

#include "expression.h"
#include "unary_operator.h"

namespace mizugaki::ast::scalar {

/**
 * @brief represents unary expressions.
 * @note `6.15 <reference resolution>`
 * @note `6.26 <numeric value expression>`
 * @note `6.29 <interval value expression>`
 * @note `6.30 <boolean value expression>`
 * @see unary_operator
 */
class unary_expression final : public expression {

    using super = expression;

public:
    /// @brief the operator kind type.
    using operator_kind_type = common::regioned<unary_operator>;

    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::unary_expression;

    /**
     * @brief creates a new instance.
     * @param operator_kind the unary operator
     * @param operand the operand expression
     * @param region the node region
     */
    explicit unary_expression(
            operator_kind_type operator_kind,
            operand_type operand,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit unary_expression(unary_expression const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit unary_expression(unary_expression&& other, ::takatori::util::object_creator creator);

    [[nodiscard]] unary_expression* clone(::takatori::util::object_creator creator) const& override;
    [[nodiscard]] unary_expression* clone(::takatori::util::object_creator creator) && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the operator kind.
     * @return the operator kind
     */
    [[nodiscard]] operator_kind_type& operator_kind() noexcept;

    /// @copydoc operator_kind()
    [[nodiscard]] operator_kind_type const& operator_kind() const noexcept;

    /**
     * @brief returns the operand.
     * @return the operand
     */
    [[nodiscard]] operand_type& operand() noexcept;

    /// @copydoc operand()
    [[nodiscard]] operand_type const& operand() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(unary_expression const& a, unary_expression const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(unary_expression const& a, unary_expression const& b) noexcept;

protected:
    [[nodiscard]] bool equals(expression const& other) const noexcept override;

private:
    operator_kind_type operator_kind_;
    operand_type operand_;
};

} // namespace mizugaki::ast::scalar
