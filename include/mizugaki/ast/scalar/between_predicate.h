#pragma once

#include <optional>

#include <takatori/util/object_creator.h>

#include <mizugaki/ast/common/regioned.h>

#include "expression.h"
#include "between_operator.h"

namespace mizugaki::ast::scalar {

/**
 * @brief processes range comparison.
 * @note `8.3 <between predicate>`
 */
class between_predicate final : public expression {

    using super = expression;

public:
    /// @brief existence of `NOT` type.
    using not_type = common::regioned<bool>;

    /// @brief the operator kind type.
    using operator_kind_type = common::regioned<between_operator>;

    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::between_predicate;

    /**
     * @brief creates a new instance.
     * @param target the comparison target
     * @param left the left edge of the comparison range
     * @param right the right comparing term
     * @param is_not whether or not this declared as `NOT`
     * @param operator_kind
     * @param region the node region
     */
    explicit between_predicate(
            operand_type target,
            operand_type left,
            operand_type right,
            std::optional<operator_kind_type> operator_kind = {},
            not_type is_not = {},
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit between_predicate(between_predicate const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit between_predicate(between_predicate&& other, ::takatori::util::object_creator creator);

    [[nodiscard]] between_predicate* clone(::takatori::util::object_creator creator) const& override;
    [[nodiscard]] between_predicate* clone(::takatori::util::object_creator creator) && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the comparison target.
     * @return the comparison target
     */
    [[nodiscard]] operand_type& target() noexcept;

    /// @copydoc target()
    [[nodiscard]] operand_type const& target() const noexcept;

    /**
     * @brief returns the left edge of comparison range.
     * @return the left edge
     */
    [[nodiscard]] operand_type& left() noexcept;

    /// @copydoc left()
    [[nodiscard]] operand_type const& left() const noexcept;

    /**
     * @brief returns the right edge of comparison range.
     * @return the right edge
     */
    [[nodiscard]] operand_type& right() noexcept;

    /// @copydoc right()
    [[nodiscard]] operand_type const& right() const noexcept;

    /**
     * @brief returns the operator kind.
     * @return the operator kind
     */
    [[nodiscard]] std::optional<operator_kind_type>& operator_kind() noexcept;

    /// @copydoc operator_kind()
    [[nodiscard]] std::optional<operator_kind_type> const& operator_kind() const noexcept;

    /**
     * @brief returns whether or not this declared as `NOT`.
     * @return true if `NOT` is declared
     * @return false otherwise
     */
    [[nodiscard]] not_type& is_not() noexcept;

    /// @copydoc is_not()
    [[nodiscard]] not_type const& is_not() const noexcept;

private:
    operand_type target_;
    operand_type left_;
    operand_type right_;
    std::optional<operator_kind_type> operator_kind_;
    not_type is_not_;
};

} // namespace mizugaki::ast::scalar