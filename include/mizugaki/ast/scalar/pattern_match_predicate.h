#pragma once

#include <takatori/util/object_creator.h>

#include <mizugaki/ast/common/regioned.h>

#include "expression.h"
#include "pattern_match_operator.h"

namespace mizugaki::ast::scalar {

/**
 * @brief processes a pattern-match comparison.
 * @note `8.5 <like predicate>`
 * @note `8.6 <similar predicate>`
 * @see pattern_match_operator
 */
class pattern_match_predicate final : public expression {

    using super = expression;

public:
    /// @brief the operator kind type.
    using operator_kind_type = common::regioned<pattern_match_operator>;

    /// @brief existence of `NOT` type.
    using not_type = common::regioned<bool>;

    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::pattern_match_predicate;

    /**
     * @brief creates a new instance.
     * @param operator_kind the pattern match operator
     * @param match_value the match value
     * @param pattern the pattern string
     * @param escape the escape character
     * @param is_not whether or not `NOT` is declared
     * @param region the node region
     */
    explicit pattern_match_predicate(
            operator_kind_type operator_kind,
            operand_type match_value,
            operand_type pattern,
            operand_type escape = {},
            not_type is_not = {},
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit pattern_match_predicate(pattern_match_predicate const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit pattern_match_predicate(pattern_match_predicate&& other, ::takatori::util::object_creator creator);

    [[nodiscard]] pattern_match_predicate* clone(::takatori::util::object_creator creator) const& override;
    [[nodiscard]] pattern_match_predicate* clone(::takatori::util::object_creator creator) && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the operator kind.
     * @return the operator kind
     */
    [[nodiscard]] operator_kind_type& operator_kind() noexcept;

    /// @copydoc operator_kind()
    [[nodiscard]] operator_kind_type const& operator_kind() const noexcept;

    /**
     * @brief returns the match value.
     * @return the match value
     */
    [[nodiscard]] operand_type& match_value() noexcept;

    /// @copydoc match_value()
    [[nodiscard]] operand_type const& match_value() const noexcept;

    /**
     * @brief returns the search pattern.
     * @return the search pattern
     */
    [[nodiscard]] operand_type& pattern() noexcept;

    /// @copydoc pattern()
    [[nodiscard]] operand_type const& pattern() const noexcept;

    /**
     * @brief returns the escape character.
     * @return the escape character
     */
    [[nodiscard]] operand_type& escape() noexcept;

    /// @copydoc escape()
    [[nodiscard]] operand_type const& escape() const noexcept;

    /**
     * @brief returns whether or not this declared as `NOT`.
     * @return true if `NOT` is declared
     * @return false otherwise
     */
    [[nodiscard]] not_type& is_not() noexcept;

    /// @copydoc is_not()
    [[nodiscard]] not_type const& is_not() const noexcept;

private:
    operator_kind_type operator_kind_;
    operand_type match_value_;
    operand_type pattern_;
    operand_type escape_;
    not_type is_not_;
};

} // namespace mizugaki::ast::scalar
