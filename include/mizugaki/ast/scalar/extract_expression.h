#pragma once

#include <takatori/util/object_creator.h>

#include <mizugaki/ast/common/regioned.h>

#include "expression.h"
#include "extract_field_kind.h"

namespace mizugaki::ast::scalar {

/**
 * @brief represents `EXTRACT` expression.
 * @note `6.17 <numeric value function>` - `<extract expression>`
 * @see extract_field_kind
 */
class extract_expression final : public expression {

    using super = expression;

public:
    /// @brief the operator kind type.
    using field_type = common::regioned<extract_field_kind>;

    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::extract_expression;

    /**
     * @brief creates a new instance.
     * @param field the target field
     * @param operand the operand expression
     * @param region the node region
     */
    explicit extract_expression(
            field_type field,
            operand_type operand,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param field the target field
     * @param operand the operand expression
     * @param region the node region
     * @attention this will take a copy of argument
     */
    explicit extract_expression(
            field_type field,
            expression&& operand,
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit extract_expression(extract_expression const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit extract_expression(extract_expression&& other, ::takatori::util::object_creator creator);

    [[nodiscard]] extract_expression* clone(::takatori::util::object_creator creator) const& override;
    [[nodiscard]] extract_expression* clone(::takatori::util::object_creator creator) && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the target field.
     * @return the target field
     */
    [[nodiscard]] field_type& field() noexcept;

    /// @copydoc field()
    [[nodiscard]] field_type const& field() const noexcept;

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
    friend bool operator==(extract_expression const& a, extract_expression const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(extract_expression const& a, extract_expression const& b) noexcept;

protected:
    [[nodiscard]] bool equals(expression const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    field_type field_;
    operand_type operand_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, extract_expression const& value);

} // namespace mizugaki::ast::scalar
