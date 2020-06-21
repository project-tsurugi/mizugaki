#pragma once

#include <takatori/util/object_creator.h>

#include <mizugaki/ast/common/regioned.h>
#include <mizugaki/ast/query/expression.h>

#include "expression.h"

namespace mizugaki::ast::scalar {

/**
 * @brief processes a quantified comparison.
 * @note `8.4 <in predicate>`
 */
class in_predicate final : public expression {

    using super = expression;

public:
    /// @brief truth type with element region information.
    using bool_type = common::regioned<bool>;

    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::in_predicate;

    /**
     * @brief creates a new instance.
     * @param left the left term
     * @param is_not whether or not `NOT` is declared
     * @param right the right term
     * @param region the node region
     */
    explicit in_predicate(
            operand_type left,
            bool_type is_not,
            ::takatori::util::unique_object_ptr<query::expression> right,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit in_predicate(in_predicate const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit in_predicate(in_predicate&& other, ::takatori::util::object_creator creator);

    [[nodiscard]] in_predicate* clone(::takatori::util::object_creator creator) const& override;
    [[nodiscard]] in_predicate* clone(::takatori::util::object_creator creator) && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

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
     * @brief returns whether or not this declared as `NOT`.
     * @return true if `NOT` is declared
     * @return false otherwise
     */
    [[nodiscard]] bool_type& is_not() noexcept;

    /// @copydoc is_not()
    [[nodiscard]] bool_type const& is_not() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(in_predicate const& a, in_predicate const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(in_predicate const& a, in_predicate const& b) noexcept;

protected:
    [[nodiscard]] bool equals(expression const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    operand_type left_;
    bool_type is_not_;
    ::takatori::util::unique_object_ptr<query::expression> right_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, in_predicate const& value);

} // namespace mizugaki::ast::scalar
