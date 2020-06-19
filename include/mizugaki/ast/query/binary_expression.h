#pragma once

#include <optional>

#include <takatori/util/object_creator.h>

#include <mizugaki/ast/common/regioned.h>
#include <mizugaki/ast/common/vector.h>
#include <mizugaki/ast/name/simple.h>
#include <mizugaki/ast/scalar/set_quantifier.h>

#include "expression.h"
#include "binary_operator.h"
#include "corresponding_clause.h"

namespace mizugaki::ast::query {

using scalar::set_quantifier;

/**
 * @brief processes dyadic operator.
 * @note `7.12 <query expression>` - `<non-join query expression>` and `<non-join query term>`
 * @see binary_operator
 */
class binary_expression final : public expression {

    using super = expression;

public:
    /// @brief the operator kind type.
    using operator_kind_type = common::regioned<binary_operator>;

    /// @brief the set quantifier type.
    using quantifier_type = common::regioned<set_quantifier>;

    /// @brief the corresponding clause type.
    using corresponding_type = corresponding_clause;

    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::binary_expression;

    /**
     * @brief creates a new instance.
     * @param left the left term
     * @param operator_kind the binary operator
     * @param quantifier the set quantifier
     * @param corresponding the corresponding clause
     * @param right the right term
     * @param region the node region
     */
    explicit binary_expression(
            ::takatori::util::unique_object_ptr<expression> left,
            operator_kind_type operator_kind,
            std::optional<quantifier_type> quantifier,
            std::optional<corresponding_type> corresponding,
            ::takatori::util::unique_object_ptr<expression> right,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit binary_expression(binary_expression const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit binary_expression(binary_expression&& other, ::takatori::util::object_creator creator);

    [[nodiscard]] binary_expression* clone(::takatori::util::object_creator creator) const& override;
    [[nodiscard]] binary_expression* clone(::takatori::util::object_creator creator) && override;

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
    [[nodiscard]] ::takatori::util::unique_object_ptr<expression>& left() noexcept;

    /// @copydoc left()
    [[nodiscard]] ::takatori::util::unique_object_ptr<expression> const& left() const noexcept;

    /**
     * @brief returns the right term.
     * @return the right term
     */
    [[nodiscard]] ::takatori::util::unique_object_ptr<expression>& right() noexcept;

    /// @copydoc right()
    [[nodiscard]] ::takatori::util::unique_object_ptr<expression> const& right() const noexcept;

    /**
     * @brief returns the set quantifier.
     * @return the set quantifier
     * @return empty if it is not declared
     */
    [[nodiscard]] std::optional<quantifier_type>& quantifier() noexcept;

    /// @copydoc quantifier()
    [[nodiscard]] std::optional<quantifier_type> const& quantifier() const noexcept;

    /**
     * @brief returns the corresponding clause (`CORRESPONDING BY`).
     * @return the corresponding clause
     * @return empty if it is not declared
     */
    [[nodiscard]] std::optional<corresponding_type>& corresponding() noexcept;

    /// @copydoc corresponding()
    [[nodiscard]] std::optional<corresponding_type> const& corresponding() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(binary_expression const& a, binary_expression const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(binary_expression const& a, binary_expression const& b) noexcept;

protected:
    [[nodiscard]] bool equals(expression const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    ::takatori::util::unique_object_ptr<expression> left_;
    operator_kind_type operator_kind_;
    std::optional<quantifier_type> quantifier_;
    std::optional<corresponding_type> corresponding_;
    ::takatori::util::unique_object_ptr<expression> right_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, binary_expression const& value);

} // namespace mizugaki::ast::query
