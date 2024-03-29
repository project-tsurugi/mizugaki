#pragma once

#include <vector>

#include <takatori/util/clone_tag.h>

#include <mizugaki/ast/common/vector.h>

#include "expression.h"
#include "case_when_clause.h"

namespace mizugaki::ast::scalar {

/**
 * @brief represents `case` expressions.
 * @note `6.21 <case expression>`
 * @note This does not represent abbreviations of case expressions.
 */
class case_expression final : public expression {

    using super = expression;

public:
    /// @brief the `WHEN` clause.
    using when_clause = case_when_clause;

    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::case_expression;

    /**
     * @brief creates a new instance.
     * @param operand the case operand
     * @param when_clauses the when clauses
     * @param default_result the default result
     * @param region the node region
     */
    explicit case_expression(
            operand_type operand,
            std::vector<when_clause> when_clauses,
            operand_type default_result = {},
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param operand the case operand
     * @param when_clauses the when clauses
     * @param default_result the default result
     * @param region the node region
     * @attention this will take copy of arguments
     */
    explicit case_expression(
            expression&& operand,
            std::initializer_list<when_clause> when_clauses,
            ::takatori::util::rvalue_ptr<expression> default_result = {},
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param when_clauses the when clauses
     * @param default_result the default result
     * @param region the node region
     * @attention this will take copy of arguments
     */
    case_expression(
            std::initializer_list<when_clause> when_clauses,
            ::takatori::util::rvalue_ptr<expression> default_result = {},
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit case_expression(::takatori::util::clone_tag_t, case_expression const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit case_expression(::takatori::util::clone_tag_t, case_expression&& other);

    [[nodiscard]] case_expression* clone() const& override;
    [[nodiscard]] case_expression* clone() && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the case operand.
     * @return the case operand.
     * @return empty if this is `<searched case>`
     */
    [[nodiscard]] operand_type& operand() noexcept;

    /// @copydoc operand()
    [[nodiscard]] operand_type const& operand() const noexcept;

    /**
     * @brief returns the list of `<when clause>`.
     * @return the when clauses
     */
    [[nodiscard]] std::vector<when_clause>& when_clauses() noexcept;

    /// @copydoc when_clauses()
    [[nodiscard]] std::vector<when_clause> const& when_clauses() const noexcept;

    /**
     * @brief returns the default result.
     * @return the default result
     * @return empty if it is not specified
     */
    [[nodiscard]] operand_type& default_result() noexcept;

    /// @copydoc default_result()
    [[nodiscard]] operand_type const& default_result() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(case_expression const& a, case_expression const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(case_expression const& a, case_expression const& b) noexcept;

protected:
    [[nodiscard]] bool equals(expression const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    operand_type operand_;
    std::vector<when_clause> when_clauses_;
    operand_type default_result_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, case_expression const& value);

} // namespace mizugaki::ast::scalar
