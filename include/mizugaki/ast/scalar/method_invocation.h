#pragma once

#include <takatori/util/object_creator.h>

#include <mizugaki/ast/common/regioned.h>
#include <mizugaki/ast/common/vector.h>
#include <mizugaki/ast/name/simple.h>

#include "expression.h"
#include "reference_operator.h"

namespace mizugaki::ast::scalar {

/**
 * @brief method on user-defined types.
 * @note `6.10 <method reference>`
 * @note `6.11 <method invocation>`
 */
class method_invocation final : public expression {

    using super = expression;

public:
    /// @brief the operator kind type.
    using operator_kind_type = common::regioned<reference_operator>;

    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::method_invocation;

    /**
     * @brief creates a new instance.
     * @param operator_kind the reference operator
     * @param value the receiver expression
     * @param name the method name
     * @param arguments the method arguments
     * @param region the node region
     */
    explicit method_invocation(
            operator_kind_type operator_kind,
            operand_type value,
            ::takatori::util::unique_object_ptr<name::simple> name,
            common::vector<operand_type> arguments,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit method_invocation(method_invocation const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit method_invocation(method_invocation&& other, ::takatori::util::object_creator creator);

    [[nodiscard]] method_invocation* clone(::takatori::util::object_creator creator) const& override;
    [[nodiscard]] method_invocation* clone(::takatori::util::object_creator creator) && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the operator kind.
     * @return the operator kind
     */
    [[nodiscard]] operator_kind_type& operator_kind() noexcept;

    /// @copydoc operator_kind()
    [[nodiscard]] operator_kind_type const& operator_kind() const noexcept;

    /**
     * @brief returns the receiver value of the target method.
     * @return the receiver value
     */
    [[nodiscard]] operand_type& value() noexcept;

    /// @copydoc value()
    [[nodiscard]] operand_type const& value() const noexcept;

    /**
     * @brief returns the method name.
     * @return the method name
     */
    [[nodiscard]] ::takatori::util::unique_object_ptr<name::simple>& name() noexcept;

    /// @brief name()
    [[nodiscard]] ::takatori::util::unique_object_ptr<name::simple> const& name() const noexcept;

    /**
     * @brief returns the function argument list.
     * @return the function argument list
     */
    [[nodiscard]] common::vector<operand_type>& arguments() noexcept;

    /// @copydoc arguments()
    [[nodiscard]] common::vector<operand_type> const& arguments() const noexcept;

private:
    operator_kind_type operator_kind_;
    operand_type value_;
    ::takatori::util::unique_object_ptr<name::simple> name_;
    common::vector<operand_type> arguments_;
};

} // namespace mizugaki::ast::scalar
