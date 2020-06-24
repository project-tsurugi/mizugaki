#pragma once

#include <takatori/util/object_creator.h>

#include <mizugaki/ast/common/regioned.h>
#include <mizugaki/ast/common/rvalue_list.h>
#include <mizugaki/ast/common/vector.h>
#include <mizugaki/ast/name/simple.h>
#include <mizugaki/ast/type/type.h>

#include "expression.h"

namespace mizugaki::ast::scalar {

/**
 * @brief static method on user-defined types.
 * @note `6.12 <static method invocation>`
 */
class static_method_invocation final : public expression {

    using super = expression;

public:
    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::static_method_invocation;

    /**
     * @brief creates a new instance.
     * @param type the declaring type
     * @param name the method name
     * @param arguments the method arguments
     * @param region the node region
     */
    explicit static_method_invocation(
            ::takatori::util::unique_object_ptr<type::type> type,
            ::takatori::util::unique_object_ptr<name::simple> name,
            common::vector<operand_type> arguments,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param type the declaring type
     * @param name the method name
     * @param arguments the method arguments
     * @param region the node region
     * @attention this will take copy of arguments
     */
    explicit static_method_invocation(
            type::type&& type,
            name::simple&& name,
            common::rvalue_list<expression> arguments,
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit static_method_invocation(static_method_invocation const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit static_method_invocation(static_method_invocation&& other, ::takatori::util::object_creator creator);

    [[nodiscard]] static_method_invocation* clone(::takatori::util::object_creator creator) const& override;
    [[nodiscard]] static_method_invocation* clone(::takatori::util::object_creator creator) && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the declaring type.
     * @return the declaring type
     */
    [[nodiscard]] ::takatori::util::unique_object_ptr<type::type>& type() noexcept;

    /// @brief type()
    [[nodiscard]] ::takatori::util::unique_object_ptr<type::type> const& type() const noexcept;

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

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(static_method_invocation const& a, static_method_invocation const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(static_method_invocation const& a, static_method_invocation const& b) noexcept;

protected:
    [[nodiscard]] bool equals(expression const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    ::takatori::util::unique_object_ptr<type::type> type_;
    ::takatori::util::unique_object_ptr<name::simple> name_;
    common::vector<operand_type> arguments_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, static_method_invocation const& value);

} // namespace mizugaki::ast::scalar
