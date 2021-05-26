#pragma once

#include <takatori/util/clone_tag.h>

#include <mizugaki/ast/common/regioned.h>
#include <mizugaki/ast/common/rvalue_list.h>
#include <mizugaki/ast/common/vector.h>
#include <mizugaki/ast/name/name.h>

#include "expression.h"

namespace mizugaki::ast::scalar {

/**
 * @brief user-defined function invocation.
 * @note `10.4 <routine invocation>`
 */
class function_invocation final : public expression {

    using super = expression;

public:
    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::function_invocation;

    /**
     * @brief creates a new instance.
     * @param name the function name
     * @param arguments the function arguments
     * @param region the node region
     */
    explicit function_invocation(
            std::unique_ptr<name::name> name,
            std::vector<operand_type> arguments,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param name the function name
     * @param arguments the function arguments
     * @param region the node region
     * @attention this will take
     */
    explicit function_invocation(
            name::name&& name,
            common::rvalue_list<expression> arguments,
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit function_invocation(::takatori::util::clone_tag_t, function_invocation const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit function_invocation(::takatori::util::clone_tag_t, function_invocation&& other);

    [[nodiscard]] function_invocation* clone() const& override;
    [[nodiscard]] function_invocation* clone() && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the function name.
     * @return the function name
     */
    [[nodiscard]] std::unique_ptr<name::name>& name() noexcept;

    /// @brief name()
    [[nodiscard]] std::unique_ptr<name::name> const& name() const noexcept;

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
    friend bool operator==(function_invocation const& a, function_invocation const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(function_invocation const& a, function_invocation const& b) noexcept;

protected:
    [[nodiscard]] bool equals(expression const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    std::unique_ptr<name::name> name_;
    std::vector<operand_type> arguments_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, function_invocation const& value);

} // namespace mizugaki::ast::scalar
