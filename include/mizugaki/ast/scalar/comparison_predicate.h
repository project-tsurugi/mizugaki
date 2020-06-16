#pragma once

#include <optional>

#include <takatori/util/object_creator.h>

#include <mizugaki/ast/common/regioned.h>

#include "expression.h"
#include "comparison_operator.h"
#include "quantifier.h"

namespace mizugaki::ast::scalar {

/**
 * @brief compares two values.
 * @note `8.2 <comparison predicate>`
 * @note `8.8 <quantified comparison predicate>`
 * @see comparison_operator
 */
class comparison_predicate final : public expression {

    using super = expression;

public:
    /// @brief the operator kind type.
    using operator_kind_type = common::regioned<comparison_operator>;

    /// @brief the quantifier type.
    using quantifier_type = common::regioned<scalar::quantifier>;

    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::comparison_predicate;

    /**
     * @brief creates a new instance.
     * @param left the left term
     * @param operator_kind the comparison operator
     * @param quantifier the quantifier
     * @param right the right term
     * @param region the node region
     */
    explicit comparison_predicate(
            operand_type left,
            operator_kind_type operator_kind,
            std::optional<quantifier_type> quantifier,
            operand_type right,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit comparison_predicate(comparison_predicate const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit comparison_predicate(comparison_predicate&& other, ::takatori::util::object_creator creator);

    [[nodiscard]] comparison_predicate* clone(::takatori::util::object_creator creator) const& override;
    [[nodiscard]] comparison_predicate* clone(::takatori::util::object_creator creator) && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the operator kind.
     * @return the operator kind
     */
    [[nodiscard]] operator_kind_type& operator_kind() noexcept;

    /// @copydoc operator_kind()
    [[nodiscard]] operator_kind_type const& operator_kind() const noexcept;

    /**
     * @brief returns the left term.
     * @return the left term
     */
    [[nodiscard]] operand_type& left() noexcept;

    /// @copydoc left()
    [[nodiscard]] operand_type const& left() const noexcept;

    /**
     * @brief returns the right term.
     * @return the right term
     */
    [[nodiscard]] operand_type& right() noexcept;

    /// @copydoc right()
    [[nodiscard]] operand_type const& right() const noexcept;

    /**
     * @brief returns the quantifier.
     * @return the quantifier
     * @return empty if it is not declared
     */
    [[nodiscard]] std::optional<quantifier_type>& quantifier() noexcept;

    /// @copydoc quantifier()
    [[nodiscard]] std::optional<quantifier_type> const& quantifier() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(comparison_predicate const& a, comparison_predicate const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(comparison_predicate const& a, comparison_predicate const& b) noexcept;

protected:
    [[nodiscard]] bool equals(expression const& other) const noexcept override;

private:
    operand_type left_;
    operator_kind_type operator_kind_;
    std::optional<quantifier_type> quantifier_;
    operand_type right_;
};

} // namespace mizugaki::ast::scalar
