#pragma once

#include <takatori/util/clone_tag.h>

#include <mizugaki/ast/common/regioned.h>
#include <mizugaki/ast/type/type.h>

#include "expression.h"
#include "cast_operator.h"

namespace mizugaki::ast::scalar {

/**
 * @brief processes type casting.
 * @note `6.22 <cast specification>`
 * @note `6.25 <subtype treatment>`
 * @note `10.4 <routine invocation>`
 */
class cast_expression final : public expression {

    using super = expression;

public:
    /// @brief the operator kind type.
    using operator_kind_type = common::regioned<cast_operator>;

    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::cast_expression;

    /**
     * @brief creates a new instance.
     * @param operator_kind the operator kind
     * @param operand the operand
     * @param type the target type
     * @param region the node region
     */
    explicit cast_expression(
            operator_kind_type operator_kind,
            operand_type operand,
            std::unique_ptr<type::type> type,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param operator_kind the operator kind
     * @param operand the operand
     * @param type the target type
     * @param region the node region
     * @attention this will take copy of arguments
     */
    explicit cast_expression(
            operator_kind_type operator_kind,
            expression&& operand,
            type::type&& type,
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit cast_expression(::takatori::util::clone_tag_t, cast_expression const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit cast_expression(::takatori::util::clone_tag_t, cast_expression&& other);

    [[nodiscard]] cast_expression* clone() const& override;
    [[nodiscard]] cast_expression* clone() && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the operator kind.
     * @return the operator kind
     */
    [[nodiscard]] operator_kind_type& operator_kind() noexcept;

    /// @brief operator_kind()
    [[nodiscard]] operator_kind_type const& operator_kind() const noexcept;

    /**
     * @brief returns the cast operand.
     * @return the cast operand
     */
    [[nodiscard]] operand_type& operand() noexcept;

    /// @brief type()
    [[nodiscard]] operand_type const& operand() const noexcept;

    /**
     * @brief returns the cast target type.
     * @return the cast target type
     */
    [[nodiscard]] std::unique_ptr<type::type>& type() noexcept;

    /// @brief type()
    [[nodiscard]] std::unique_ptr<type::type> const& type() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(cast_expression const& a, cast_expression const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(cast_expression const& a, cast_expression const& b) noexcept;

protected:
    [[nodiscard]] bool equals(expression const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    operator_kind_type operator_kind_;
    operand_type operand_;
    std::unique_ptr<type::type> type_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, cast_expression const& value);

} // namespace mizugaki::ast::scalar
