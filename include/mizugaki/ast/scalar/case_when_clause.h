#pragma once

#include <mizugaki/ast/common/clone_wrapper.h>

#include "expression.h"

namespace mizugaki::ast::scalar {

/**
 * @brief represents `case` expressions.
 * @note `6.21 <case expression>`
 * @note This does not represent abbreviations of case expressions.
 */
class case_when_clause final : public element {
public:
    /// @brief the common operand type.
    using operand_type = expression::operand_type;

    /**
     * @brief creates a new instance.
     * @param when the `when` operand
     * @param result the result of this clause
     * @param region the node region
     */
    explicit case_when_clause(
            operand_type when,
            operand_type result,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit case_when_clause(case_when_clause const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit case_when_clause(case_when_clause&& other, ::takatori::util::object_creator creator);

    /**
     * @brief returns the operand of when clause.
     * @return the operand of when clause
     */
    [[nodiscard]] operand_type& when() noexcept;

    /// @copydoc when()
    [[nodiscard]] operand_type const& when() const noexcept;

    /**
     * @brief returns the result of when clause.
     * @return the result of when clause
     */
    [[nodiscard]] operand_type& result() noexcept;

    /// @copydoc result()
    [[nodiscard]] operand_type const& result() const noexcept;
    
    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(case_when_clause const& a, case_when_clause const& b) noexcept;
    
    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(case_when_clause const& a, case_when_clause const& b) noexcept;

private:
    common::clone_wrapper<::takatori::util::unique_object_ptr<expression>> when_;
    common::clone_wrapper<::takatori::util::unique_object_ptr<expression>> result_;
};

} // namespace mizugaki::ast::scalar
