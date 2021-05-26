#pragma once

#include <takatori/util/clone_tag.h>

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
     * @param operator_kind the unary operator
     * @param operand the operand expression
     * @param region the node region
     * @attention this will take a copy of argument
     */
    explicit unary_expression(
            operator_kind_type operator_kind,
            expression&& operand,
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit unary_expression(::takatori::util::clone_tag_t, unary_expression const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit unary_expression(::takatori::util::clone_tag_t, unary_expression&& other);

    [[nodiscard]] unary_expression* clone() const& override;
    [[nodiscard]] unary_expression* clone() && override;

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
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    operator_kind_type operator_kind_;
    operand_type operand_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, unary_expression const& value);

} // namespace mizugaki::ast::scalar
