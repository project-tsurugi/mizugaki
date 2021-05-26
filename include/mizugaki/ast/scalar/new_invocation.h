#pragma once

#include <takatori/util/clone_tag.h>

#include <mizugaki/ast/common/regioned.h>
#include <mizugaki/ast/common/rvalue_list.h>
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
            std::unique_ptr<type::type> type,
            std::vector<operand_type> arguments,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param type the target type
     * @param arguments the function arguments
     * @param region the node region
     * @attention this will take copy of arguments
     */
    explicit new_invocation(
            type::type&& type,
            common::rvalue_list<expression> arguments,
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit new_invocation(::takatori::util::clone_tag_t, new_invocation const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit new_invocation(::takatori::util::clone_tag_t, new_invocation&& other);

    [[nodiscard]] new_invocation* clone() const& override;
    [[nodiscard]] new_invocation* clone() && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the target type.
     * @return the target type
     */
    [[nodiscard]] std::unique_ptr<type::type>& type() noexcept;

    /// @brief type()
    [[nodiscard]] std::unique_ptr<type::type> const& type() const noexcept;

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
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    std::unique_ptr<type::type> type_;
    std::vector<operand_type> arguments_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, new_invocation const& value);

} // namespace mizugaki::ast::scalar
