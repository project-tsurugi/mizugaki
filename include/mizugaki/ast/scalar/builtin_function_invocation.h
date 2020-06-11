#pragma once

#include <takatori/util/object_creator.h>

#include <mizugaki/ast/common/regioned.h>
#include <mizugaki/ast/common/vector.h>

#include "expression.h"
#include "builtin_function_kind.h"

namespace mizugaki::ast::scalar {

/**
 * @brief represents predefined built-in function.
 * @note `6.17 <numeric value function>`
 * @note `6.18 <string value function>`
 * @note `6.19 <datetime value function>`
 * @note `6.20 <interval value function>`
 * @see builtin_function_kind
 */
class builtin_function_invocation final : public expression {

    using super = expression;

public:
    /// @brief the function symbol type.
    using function_type = common::regioned<builtin_function_kind>;

    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::builtin_function_invocation;

    /**
     * @brief creates a new instance.
     * @param function the function symbol
     * @param arguments the function arguments
     * @param region the node region
     */
    explicit builtin_function_invocation(
            function_type function,
            common::vector<operand_type> arguments,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit builtin_function_invocation(builtin_function_invocation const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit builtin_function_invocation(builtin_function_invocation&& other, ::takatori::util::object_creator creator);

    [[nodiscard]] builtin_function_invocation* clone(::takatori::util::object_creator creator) const& override;
    [[nodiscard]] builtin_function_invocation* clone(::takatori::util::object_creator creator) && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the function symbol.
     * @return the function symbol
     */
    [[nodiscard]] function_type& function() noexcept;

    /// @brief function()
    [[nodiscard]] function_type const& function() const noexcept;

    /**
     * @brief returns the function argument list.
     * @return the function argument list
     */
    [[nodiscard]] common::vector<operand_type>& arguments() noexcept;

    /// @copydoc arguments()
    [[nodiscard]] common::vector<operand_type> const& arguments() const noexcept;

private:
    function_type function_;
    common::vector<operand_type> arguments_;
};

} // namespace mizugaki::ast::scalar