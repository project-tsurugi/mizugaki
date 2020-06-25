#pragma once

#include <optional>

#include <takatori/util/clone_tag.h>

#include <mizugaki/ast/common/regioned.h>
#include <mizugaki/ast/common/vector.h>
#include <mizugaki/ast/common/rvalue_list.h>

#include "expression.h"
#include "builtin_set_function_kind.h"
#include "set_quantifier.h"

namespace mizugaki::ast::scalar {

/**
 * @brief predefined built-in set function.
 * @note `6.16 <set function specification>`
 * @see builtin_set_function_kind
 */
class builtin_set_function_invocation final : public expression {

    using super = expression;

public:
    /// @brief the function symbol type.
    using function_type = common::regioned<builtin_set_function_kind>;

    /// @brief the quantifier type.
    using quantifier_type = common::regioned<set_quantifier>;

    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::builtin_set_function_invocation;

    /**
     * @brief creates a new instance.
     * @param function the function symbol
     * @param quantifier the set quantifier
     * @param arguments the function arguments
     * @param region the node region
     */
    explicit builtin_set_function_invocation(
            function_type function,
            std::optional<quantifier_type> quantifier,
            std::vector<operand_type> arguments,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param function the function symbol
     * @param quantifier the set quantifier
     * @param arguments the function arguments
     * @param region the node region
     * @attention this will take copy of arguments
     */
    explicit builtin_set_function_invocation(
            function_type function,
            std::optional<quantifier_type> quantifier,
            common::rvalue_list<expression> arguments,
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit builtin_set_function_invocation(::takatori::util::clone_tag_t, builtin_set_function_invocation const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit builtin_set_function_invocation(::takatori::util::clone_tag_t, builtin_set_function_invocation&& other);

    [[nodiscard]] builtin_set_function_invocation* clone() const& override;
    [[nodiscard]] builtin_set_function_invocation* clone() && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the function symbol.
     * @return the function symbol
     */
    [[nodiscard]] function_type& function() noexcept;

    /// @brief function()
    [[nodiscard]] function_type const& function() const noexcept;

    /**
     * @brief returns the quantifier.
     * @return the quantifier
     */
    [[nodiscard]] std::optional<quantifier_type>& quantifier() noexcept;

    /// @copydoc quantifier()
    [[nodiscard]] std::optional<quantifier_type> const& quantifier() const noexcept;

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
    friend bool operator==(builtin_set_function_invocation const& a, builtin_set_function_invocation const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(builtin_set_function_invocation const& a, builtin_set_function_invocation const& b) noexcept;

protected:
    [[nodiscard]] bool equals(expression const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    function_type function_;
    std::optional<quantifier_type> quantifier_;
    std::vector<operand_type> arguments_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, builtin_set_function_invocation const& value);

} // namespace mizugaki::ast::scalar
