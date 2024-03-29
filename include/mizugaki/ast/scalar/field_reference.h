#pragma once

#include <takatori/util/clone_tag.h>

#include <mizugaki/ast/common/regioned.h>
#include <mizugaki/ast/name/simple.h>

#include "expression.h"
#include "reference_operator.h"

namespace mizugaki::ast::scalar {

/**
 * @brief refers fields.
 * @note `6.8 <field reference>`
 * @note `6.14 <dereference operation>`
 */
class field_reference final : public expression {

    using super = expression;

public:
    /// @brief the operator kind type.
    using operator_kind_type = common::regioned<reference_operator>;

    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::field_reference;

    /**
     * @brief creates a new instance.
     * @param value the field owner expression
     * @param operator_kind the reference operator kind
     * @param name the field name
     * @param region the node region
     */
    explicit field_reference(
            operand_type value,
            operator_kind_type operator_kind,
            std::unique_ptr<name::simple> name,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param value the field owner expression
     * @param name the field name
     * @param operator_kind the reference operator kind
     * @param region the node region
     */
    explicit field_reference(
            expression&& value,
            name::simple&& name,
            operator_kind_type operator_kind = reference_operator::period,
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit field_reference(::takatori::util::clone_tag_t, field_reference const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit field_reference(::takatori::util::clone_tag_t, field_reference&& other);

    [[nodiscard]] field_reference* clone() const& override;
    [[nodiscard]] field_reference* clone() && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the operator kind.
     * @return the operator kind
     */
    [[nodiscard]] operator_kind_type& operator_kind() noexcept;

    /// @copydoc operator_kind()
    [[nodiscard]] operator_kind_type const& operator_kind() const noexcept;

    /**
     * @brief returns the receiver value or reference of the target field.
     * @return the receiver value
     */
    [[nodiscard]] expression::operand_type& value() noexcept;

    /// @copydoc value()
    [[nodiscard]] expression::operand_type const& value() const noexcept;

    /**
     * @brief returns the field name.
     * @return the field name
     */
    [[nodiscard]] std::unique_ptr<name::simple>& name() noexcept;

    /// @brief name()
    [[nodiscard]] std::unique_ptr<name::simple> const& name() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(field_reference const& a, field_reference const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(field_reference const& a, field_reference const& b) noexcept;

protected:
    [[nodiscard]] bool equals(expression const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    operand_type value_;
    operator_kind_type operator_kind_;
    std::unique_ptr<name::simple> name_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, field_reference const& value);

} // namespace mizugaki::ast::scalar
