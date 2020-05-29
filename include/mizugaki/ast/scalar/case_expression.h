#pragma once

#include <vector>

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
            common::vector<when_clause> when_clauses,
            operand_type default_result = {},
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit case_expression(case_expression const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit case_expression(case_expression&& other, ::takatori::util::object_creator creator);

    [[nodiscard]] case_expression* clone(::takatori::util::object_creator creator) const& override;
    [[nodiscard]] case_expression* clone(::takatori::util::object_creator creator) && override;

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
    [[nodiscard]] common::vector<when_clause>& when_clauses() noexcept;

    /// @copydoc when_clauses()
    [[nodiscard]] common::vector<when_clause> const& when_clauses() const noexcept;

    /**
     * @brief returns the default result.
     * @return the default result
     * @return empty if it is not specified
     */
    [[nodiscard]] operand_type& default_result() noexcept;

    /// @copydoc default_result()
    [[nodiscard]] operand_type const& default_result() const noexcept;

private:
    operand_type operand_;
    common::vector<when_clause> when_clauses_;
    operand_type default_result_;
};

} // namespace mizugaki::ast::scalar
