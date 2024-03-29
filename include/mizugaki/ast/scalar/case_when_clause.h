#pragma once

#include <takatori/util/clone_tag.h>

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
     * @brief this will be used in parser generator
     */
    explicit case_when_clause() = default;

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
     * @param when the `when` operand
     * @param result the result of this clause
     * @param region the node region
     * @attention this will take copy of arguments
     */
    case_when_clause(
            expression&& when,
            expression&& result,
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit case_when_clause(::takatori::util::clone_tag_t, case_when_clause const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit case_when_clause(::takatori::util::clone_tag_t, case_when_clause&& other);

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

    /**
     * @brief dumps structure information of the given value into the target acceptor.
     * @param acceptor the target acceptor
     * @param value the target value
     * @return the output
     */
    friend ::takatori::serializer::object_acceptor& operator<<(
            ::takatori::serializer::object_acceptor& acceptor,
            case_when_clause const& value);

private:
    common::clone_wrapper<std::unique_ptr<expression>> when_;
    common::clone_wrapper<std::unique_ptr<expression>> result_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, case_when_clause const& value);

} // namespace mizugaki::ast::scalar
