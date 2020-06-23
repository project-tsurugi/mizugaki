#pragma once

#include <takatori/util/object_creator.h>

#include <mizugaki/ast/common/regioned.h>
#include <mizugaki/ast/query/expression.h>

#include "expression.h"
#include "table_operator.h"

namespace mizugaki::ast::scalar {

/**
 * @brief tests on a table.
 * @note `8.9 <exists predicate>`
 * @note `8.10 <unique predicate>`
 * @see table_operator
 */
class table_predicate final : public expression {

    using super = expression;

public:
    /// @brief the operator kind type.
    using operator_kind_type = common::regioned<table_operator>;

    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::table_predicate;

    /**
     * @brief creates a new instance.
     * @param operator_kind the unary operator
     * @param operand the operand expression
     * @param region the node region
     */
    explicit table_predicate(
            operator_kind_type operator_kind,
            ::takatori::util::unique_object_ptr<query::expression> operand,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param operator_kind the unary operator
     * @param operand the operand expression
     * @param region the node region
     * @attention this will take copy of argument
     */
    explicit table_predicate(
            operator_kind_type operator_kind,
            query::expression&& operand,
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit table_predicate(table_predicate const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit table_predicate(table_predicate&& other, ::takatori::util::object_creator creator);

    [[nodiscard]] table_predicate* clone(::takatori::util::object_creator creator) const& override;
    [[nodiscard]] table_predicate* clone(::takatori::util::object_creator creator) && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the operator kind.
     * @return the operator kind
     */
    [[nodiscard]] operator_kind_type& operator_kind() noexcept;

    /// @copydoc operator_kind()
    [[nodiscard]] operator_kind_type const& operator_kind() const noexcept;
    /**
     * @brief returns the operand term.
     * @return the operand term
     */
    [[nodiscard]] ::takatori::util::unique_object_ptr<query::expression>& operand() noexcept;

    /// @copydoc operand()
    [[nodiscard]] ::takatori::util::unique_object_ptr<query::expression> const& operand() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(table_predicate const& a, table_predicate const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(table_predicate const& a, table_predicate const& b) noexcept;

protected:
    [[nodiscard]] bool equals(expression const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    operator_kind_type operator_kind_;
    ::takatori::util::unique_object_ptr<query::expression> operand_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, table_predicate const& value);

} // namespace mizugaki::ast::scalar
