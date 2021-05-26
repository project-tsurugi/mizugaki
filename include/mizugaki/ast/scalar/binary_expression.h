#pragma once

#include <takatori/util/clone_tag.h>

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
     * @param left the left term
     * @param operator_kind the binary operator
     * @param right the right term
     * @param region the node region
     * @attention this will take copy of arguments
     */
    explicit binary_expression(
            expression&& left,
            operator_kind_type operator_kind,
            expression&& right,
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit binary_expression(::takatori::util::clone_tag_t, binary_expression const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit binary_expression(::takatori::util::clone_tag_t, binary_expression&& other);

    [[nodiscard]] binary_expression* clone() const& override;
    [[nodiscard]] binary_expression* clone() && override;

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
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    operand_type left_;
    operator_kind_type operator_kind_;
    operand_type right_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, binary_expression const& value);

} // namespace mizugaki::ast::scalar
