#pragma once

#include <takatori/util/object_creator.h>

#include <mizugaki/ast/common/regioned.h>
#include <mizugaki/ast/name/name.h>

#include "expression.h"
#include "convert_operator.h"

namespace mizugaki::ast::scalar {

/**
 * @brief processes string conversions.
 * @note `6.18 <string value function>`
 * @see convert_operator
 */
class convert_expression final : public expression {

    using super = expression;

public:
    /// @brief the operator kind type.
    using operator_kind_type = common::regioned<convert_operator>;

    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::convert_expression;

    /**
     * @brief creates a new instance.
     * @param operator_kind the convert operator
     * @param operand the operand term
     * @param name the conversion name
     * @param region the node region
     */
    explicit convert_expression(
            operator_kind_type operator_kind,
            operand_type operand,
            ::takatori::util::unique_object_ptr<name::name> name,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit convert_expression(convert_expression const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit convert_expression(convert_expression&& other, ::takatori::util::object_creator creator);

    [[nodiscard]] convert_expression* clone(::takatori::util::object_creator creator) const& override;
    [[nodiscard]] convert_expression* clone(::takatori::util::object_creator creator) && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the operator kind.
     * @return the operator kind
     */
    [[nodiscard]] operator_kind_type& operator_kind() noexcept;

    /// @copydoc operator_kind()
    [[nodiscard]] operator_kind_type const& operator_kind() const noexcept;

    /**
     * @brief returns the operand term.
     * @return the operand term
     */
    [[nodiscard]] operand_type& operand() noexcept;

    /// @copydoc operand()
    [[nodiscard]] operand_type const& operand() const noexcept;

    /**
     * @brief returns the name term.
     * @return the name term
     */
    [[nodiscard]] ::takatori::util::unique_object_ptr<name::name>& name() noexcept;

    /// @copydoc name()
    [[nodiscard]] ::takatori::util::unique_object_ptr<name::name> const& name() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(convert_expression const& a, convert_expression const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(convert_expression const& a, convert_expression const& b) noexcept;

protected:
    [[nodiscard]] bool equals(expression const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    operator_kind_type operator_kind_;
    operand_type operand_;
    ::takatori::util::unique_object_ptr<name::name> name_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, convert_expression const& value);

} // namespace mizugaki::ast::scalar
