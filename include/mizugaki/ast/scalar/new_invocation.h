#pragma once

#include <takatori/util/object_creator.h>

#include <mizugaki/ast/common/regioned.h>
#include <mizugaki/ast/common/vector.h>
#include <mizugaki/ast/type/type.h>

#include "expression.h"

namespace mizugaki::ast::scalar {

/**
 * @brief invoke `NEW` for user-defined type.
 * @note `6.24 <new specification>`
 */
class new_invocation final : public expression {

    using super = expression;

public:
    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::new_invocation;

    /**
     * @brief creates a new instance.
     * @param type the target type
     * @param arguments the function arguments
     * @param region the node region
     */
    explicit new_invocation(
            ::takatori::util::unique_object_ptr<type::type> type,
            common::vector<operand_type> arguments,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit new_invocation(new_invocation const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit new_invocation(new_invocation&& other, ::takatori::util::object_creator creator);

    [[nodiscard]] new_invocation* clone(::takatori::util::object_creator creator) const& override;
    [[nodiscard]] new_invocation* clone(::takatori::util::object_creator creator) && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the target type.
     * @return the target type
     */
    [[nodiscard]] ::takatori::util::unique_object_ptr<type::type>& type() noexcept;

    /// @brief type()
    [[nodiscard]] ::takatori::util::unique_object_ptr<type::type> const& type() const noexcept;

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
    friend bool operator==(new_invocation const& a, new_invocation const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(new_invocation const& a, new_invocation const& b) noexcept;

protected:
    [[nodiscard]] bool equals(expression const& other) const noexcept override;

private:
    ::takatori::util::unique_object_ptr<type::type> type_;
    common::vector<operand_type> arguments_;
};

} // namespace mizugaki::ast::scalar
