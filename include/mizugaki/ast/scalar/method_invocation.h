#pragma once

#include <takatori/util/clone_tag.h>

#include <mizugaki/ast/common/regioned.h>
#include <mizugaki/ast/common/rvalue_list.h>
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
     * @param value the receiver expression
     * @param operator_kind the reference operator
     * @param name the method name
     * @param arguments the method arguments
     * @param region the node region
     */
    explicit method_invocation(
            operand_type value,
            operator_kind_type operator_kind,
            std::unique_ptr<name::simple> name,
            std::vector<operand_type> arguments,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param value the receiver expression
     * @param operator_kind the reference operator
     * @param name the method name
     * @param arguments the method arguments
     * @param region the node region
     * @attention this will take copy of arguments
     */
    explicit method_invocation(
            expression&& value,
            name::simple&& name,
            common::rvalue_list<expression> arguments,
            operator_kind_type operator_kind = reference_operator::period,
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit method_invocation(::takatori::util::clone_tag_t, method_invocation const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit method_invocation(::takatori::util::clone_tag_t, method_invocation&& other);

    [[nodiscard]] method_invocation* clone() const& override;
    [[nodiscard]] method_invocation* clone() && override;

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
    [[nodiscard]] std::unique_ptr<name::simple>& name() noexcept;

    /// @brief name()
    [[nodiscard]] std::unique_ptr<name::simple> const& name() const noexcept;

    /**
     * @brief returns the function argument list.
     * @return the function argument list
     */
    [[nodiscard]] std::vector<operand_type>& arguments() noexcept;

    /// @copydoc arguments()
    [[nodiscard]] std::vector<operand_type> const& arguments() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(method_invocation const& a, method_invocation const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(method_invocation const& a, method_invocation const& b) noexcept;

protected:
    [[nodiscard]] bool equals(expression const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    operand_type value_;
    operator_kind_type operator_kind_;
    std::unique_ptr<name::simple> name_;
    std::vector<operand_type> arguments_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, method_invocation const& value);

} // namespace mizugaki::ast::scalar
