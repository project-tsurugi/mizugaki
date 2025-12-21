#pragma once

#include <takatori/util/clone_tag.h>
#include <takatori/util/rvalue_ptr.h>

#include <mizugaki/ast/common/regioned.h>

#include "expression.h"
#include "join_type.h"
#include "join_specification.h"

namespace mizugaki::ast::table {

/**
 * @brief processes join operation.
 * @note `7.7 <joined table>`
 */
class join final : public expression {

    using super = expression;

public:
    /// @brief the join type.
    using operator_kind_type = common::regioned<join_type>;

    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::join;

    /**
     * @brief creates a new instance.
     * @param left the left operand
     * @param operator_kind the join type
     * @param right the right operand
     * @param specification the join specification (`ON`, `USING` or nothing)
     * @param region the node region
     */
    explicit join(
            std::unique_ptr<table::expression> left,
            operator_kind_type operator_kind,
            std::unique_ptr<table::expression> right,
            std::unique_ptr<join_specification> specification = {},
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param left the left operand
     * @param operator_kind the join type
     * @param right the right operand
     * @param specification the join specification (`ON`, `USING` or nothing)
     * @param region the node region
     * @attention this will take copy of arguments
     */
    explicit join(
            table::expression&& left,
            operator_kind_type operator_kind,
            table::expression&& right,
            ::takatori::util::rvalue_ptr<join_specification> specification = {},
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit join(::takatori::util::clone_tag_t, join const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit join(::takatori::util::clone_tag_t, join&& other);

    [[nodiscard]] join* clone() const& override;
    [[nodiscard]] join* clone() && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the join type.
     * @return the join type
     */
    [[nodiscard]] operator_kind_type& operator_kind() noexcept;

    /// @copydoc operator_kind()
    [[nodiscard]] operator_kind_type const& operator_kind() const noexcept;

    /**
     * @brief returns the left operand.
     * @return the left operand
     */
    [[nodiscard]] std::unique_ptr<table::expression>& left() noexcept;

    /// @copydoc left()
    [[nodiscard]] std::unique_ptr<table::expression> const& left() const noexcept;

    /**
     * @brief returns the right operand.
     * @return the right operand
     */
    [[nodiscard]] std::unique_ptr<table::expression>& right() noexcept;

    /// @copydoc right()
    [[nodiscard]] std::unique_ptr<table::expression> const& right() const noexcept;

    /**
     * @brief returns the join specification.
     * @return the join specification
     * @return empty if it is not declared
     */
    [[nodiscard]] std::unique_ptr<join_specification>& specification() noexcept;

    /// @copydoc specification()
    [[nodiscard]] std::unique_ptr<join_specification> const& specification() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(join const& a, join const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(join const& a, join const& b) noexcept;

protected:
    [[nodiscard]] bool equals(expression const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    std::unique_ptr<table::expression> left_;
    operator_kind_type operator_kind_;
    std::unique_ptr<table::expression> right_;
    std::unique_ptr<join_specification> specification_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, join const& value);

} // namespace mizugaki::ast::table
