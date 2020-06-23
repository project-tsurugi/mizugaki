#pragma once

#include <optional>

#include <takatori/util/object_creator.h>

#include <mizugaki/ast/common/regioned.h>
#include <mizugaki/ast/query/expression.h>

#include "expression.h"
#include "comparison_operator.h"
#include "quantifier.h"

namespace mizugaki::ast::scalar {

/**
 * @brief compares two values.
 * @note `8.8 <quantified comparison predicate>`
 * @see comparison_operator
 */
class quantified_comparison_predicate final : public expression {

    using super = expression;

public:
    /// @brief the operator kind type.
    using operator_kind_type = common::regioned<comparison_operator>;

    /// @brief the quantifier type.
    using quantifier_type = common::regioned<scalar::quantifier>;

    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::quantified_comparison_predicate;

    /**
     * @brief creates a new instance.
     * @param left the left term
     * @param operator_kind the comparison operator
     * @param quantifier the quantifier
     * @param right the right term
     * @param region the node region
     */
    explicit quantified_comparison_predicate(
            operand_type left,
            operator_kind_type operator_kind,
            quantifier_type quantifier,
            ::takatori::util::unique_object_ptr<query::expression> right,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param left the left term
     * @param operator_kind the comparison operator
     * @param quantifier the quantifier
     * @param right the right term
     * @param region the node region
     * @attention this will take copy of arguments
     */
    explicit quantified_comparison_predicate(
            expression&& left,
            operator_kind_type operator_kind,
            quantifier_type quantifier,
            query::expression&& right,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit quantified_comparison_predicate(quantified_comparison_predicate const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit quantified_comparison_predicate(quantified_comparison_predicate&& other, ::takatori::util::object_creator creator);

    [[nodiscard]] quantified_comparison_predicate* clone(::takatori::util::object_creator creator) const& override;
    [[nodiscard]] quantified_comparison_predicate* clone(::takatori::util::object_creator creator) && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the operator kind.
     * @return the operator kind
     */
    [[nodiscard]] operator_kind_type& operator_kind() noexcept;

    /// @copydoc operator_kind()
    [[nodiscard]] operator_kind_type const& operator_kind() const noexcept;

    /**
     * @brief returns the quantifier.
     * @return the quantifier
     * @return empty if it is not declared
     */
    [[nodiscard]] quantifier_type& quantifier() noexcept;

    /// @copydoc quantifier()
    [[nodiscard]] quantifier_type const& quantifier() const noexcept;

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
    [[nodiscard]] ::takatori::util::unique_object_ptr<query::expression>& right() noexcept;

    /// @copydoc right()
    [[nodiscard]] ::takatori::util::unique_object_ptr<query::expression> const& right() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(quantified_comparison_predicate const& a, quantified_comparison_predicate const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(quantified_comparison_predicate const& a, quantified_comparison_predicate const& b) noexcept;

protected:
    [[nodiscard]] bool equals(expression const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    operand_type left_;
    operator_kind_type operator_kind_;
    quantifier_type quantifier_;
    ::takatori::util::unique_object_ptr<query::expression> right_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, quantified_comparison_predicate const& value);

} // namespace mizugaki::ast::scalar
