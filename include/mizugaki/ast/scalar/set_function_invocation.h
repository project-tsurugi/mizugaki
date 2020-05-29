#pragma once

#include <takatori/util/object_creator.h>

#include <mizugaki/ast/common/regioned.h>
#include <mizugaki/ast/common/vector.h>
#include <mizugaki/ast/name/name.h>

#include "expression.h"
#include "set_quantifier.h"

namespace mizugaki::ast::scalar {

/**
 * @brief user-defined set function invocation.
 */
class set_function_invocation final : public expression {

    using super = expression;

public:
    /// @brief the quantifier type.
    using quantifier_type = common::regioned<set_quantifier>;

    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::set_function_invocation;

    /**
     * @brief creates a new instance.
     * @param name the function name
     * @param quantifier the set quantifier
     * @param arguments the function arguments
     * @param region the node region
     */
    explicit set_function_invocation(
            ::takatori::util::unique_object_ptr<name::name> name,
            std::optional<quantifier_type> quantifier,
            common::vector<operand_type> arguments,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit set_function_invocation(set_function_invocation const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit set_function_invocation(set_function_invocation&& other, ::takatori::util::object_creator creator);

    [[nodiscard]] set_function_invocation* clone(::takatori::util::object_creator creator) const& override;
    [[nodiscard]] set_function_invocation* clone(::takatori::util::object_creator creator) && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the function name.
     * @return the function name
     */
    [[nodiscard]] ::takatori::util::unique_object_ptr<name::name>& name() noexcept;

    /// @brief name()
    [[nodiscard]] ::takatori::util::unique_object_ptr<name::name> const& name() const noexcept;

    /**
     * @brief returns the quantifier.
     * @return the quantifier
     */
    [[nodiscard]] constexpr std::optional<quantifier_type>& quantifier() noexcept;

    /// @copydoc quantifier()
    [[nodiscard]] constexpr std::optional<quantifier_type> const& quantifier() const noexcept;

    /**
     * @brief returns the function argument list.
     * @return the function argument list
     */
    [[nodiscard]] common::vector<operand_type>& arguments() noexcept;

    /// @copydoc arguments()
    [[nodiscard]] common::vector<operand_type> const& arguments() const noexcept;

private:
    ::takatori::util::unique_object_ptr<name::name> name_;
    std::optional<quantifier_type> quantifier_;
    common::vector<operand_type> arguments_;
};

} // namespace mizugaki::ast::scalar
