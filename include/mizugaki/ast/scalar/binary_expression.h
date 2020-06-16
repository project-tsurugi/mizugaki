#pragma once

#include <takatori/util/object_creator.h>

#include <mizugaki/ast/common/regioned.h>

#include "expression.h"
#include "binary_operator.h"

namespace mizugaki::ast::scalar {

/**
 * @brief processes dyadic operators.
 * @note `6.26 <numeric value expression>`
 * @note `6.27 <string value expression>`
 * @note `6.28 <datetime value expression>`
 * @note `6.29 <interval value expression>`
 * @note `6.30 <boolean value expression>`
 * @see binary_operator
 */
class binary_expression final : public expression {

    using super = expression;

public:
    /// @brief the operator kind type.
    using operator_kind_type = common::regioned<binary_operator>;

    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::binary_expression;

    /**
     * @brief creates a new instance.
     * @param left the left term
     * @param operator_kind the binary operator
     * @param right the right term
     * @param region the node region
     */
    explicit binary_expression(
            operand_type left,
            operator_kind_type operator_kind,
            operand_type right,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit binary_expression(binary_expression const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit binary_expression(binary_expression&& other, ::takatori::util::object_creator creator);

    [[nodiscard]] binary_expression* clone(::takatori::util::object_creator creator) const& override;
    [[nodiscard]] binary_expression* clone(::takatori::util::object_creator creator) && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the operator kind.
     * @return the operator kind
     */
    [[nodiscard]] operator_kind_type& operator_kind() noexcept;

    /// @copydoc operator_kind()
    [[nodiscard]] operator_kind_type const& operator_kind() const noexcept;

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
    [[nodiscard]] operand_type& right() noexcept;

    /// @copydoc right()
    [[nodiscard]] operand_type const& right() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(binary_expression const& a, binary_expression const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(binary_expression const& a, binary_expression const& b) noexcept;

protected:
    [[nodiscard]] bool equals(expression const& other) const noexcept override;

private:
    operand_type left_;
    operator_kind_type operator_kind_;
    operand_type right_;
};

} // namespace mizugaki::ast::scalar
