#pragma once

#include <takatori/util/object_creator.h>

#include <mizugaki/ast/common/regioned.h>
#include <mizugaki/ast/common/vector.h>
#include <mizugaki/ast/common/rvalue_list.h>

#include "expression.h"
#include "value_constructor_kind.h"

namespace mizugaki::ast::scalar {

/**
 * @brief constructs a row value.
 * @note `7.1 <row value constructor>`
 */
class value_constructor final : public expression {

    using super = expression;

public:
    /// @brief the constructor kind type.
    using operator_kind_type = common::regioned<value_constructor_kind>;

    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::value_constructor;

    /**
     * @brief creates a new instance.
     * @param operator_kind the construction target kind
     * @param elements the row value elements
     * @param region the node region
     */
    explicit value_constructor(
            operator_kind_type operator_kind,
            common::vector<operand_type> elements,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param operator_kind the construction target kind
     * @param elements the row value elements
     * @param region the node region
     */
    value_constructor(
            common::rvalue_list<scalar::expression> elements,
            value_constructor_kind operator_kind = value_constructor_kind::row,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit value_constructor(value_constructor const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit value_constructor(value_constructor&& other, ::takatori::util::object_creator creator);

    [[nodiscard]] value_constructor* clone(::takatori::util::object_creator creator) const& override;
    [[nodiscard]] value_constructor* clone(::takatori::util::object_creator creator) && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the constructor kind.
     * @return the constructor kind
     */
    [[nodiscard]] operator_kind_type& operator_kind() noexcept;

    /// @copydoc operator_kind()
    [[nodiscard]] operator_kind_type const& operator_kind() const noexcept;

    /**
     * @brief returns the row value elements.
     * @return the row value elements
     */
    [[nodiscard]] common::vector<operand_type>& elements() noexcept;

    /// @copydoc elements()
    [[nodiscard]] common::vector<operand_type> const& elements() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(value_constructor const& a, value_constructor const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(value_constructor const& a, value_constructor const& b) noexcept;

protected:
    [[nodiscard]] bool equals(expression const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    operator_kind_type operator_kind_;
    common::vector<operand_type> elements_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, value_constructor const& value);

} // namespace mizugaki::ast::scalar
