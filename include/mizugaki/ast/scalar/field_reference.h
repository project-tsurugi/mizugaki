#pragma once

#include <takatori/util/object_creator.h>

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
     * @param operator_kind the reference operator kind
     * @param value the field owner expression
     * @param name the field name
     * @param region the node region
     */
    explicit field_reference(
            operator_kind_type operator_kind,
            expression::operand_type value,
            ::takatori::util::unique_object_ptr<name::simple> name,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit field_reference(field_reference const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit field_reference(field_reference&& other, ::takatori::util::object_creator creator);

    [[nodiscard]] field_reference* clone(::takatori::util::object_creator creator) const& override;
    [[nodiscard]] field_reference* clone(::takatori::util::object_creator creator) && override;

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
    [[nodiscard]] ::takatori::util::unique_object_ptr<name::simple>& name() noexcept;

    /// @brief name()
    [[nodiscard]] ::takatori::util::unique_object_ptr<name::simple> const& name() const noexcept;

private:
    operator_kind_type operator_kind_;
    operand_type value_;
    ::takatori::util::unique_object_ptr<name::simple> name_;
};

} // namespace mizugaki::ast::scalar