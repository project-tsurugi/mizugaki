#pragma once

#include <optional>

#include <takatori/util/clone_tag.h>

#include <mizugaki/ast/common/regioned.h>
#include <mizugaki/ast/common/vector.h>
#include <mizugaki/ast/common/rvalue_list.h>
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
            std::unique_ptr<expression> left,
            operator_kind_type operator_kind,
            std::optional<quantifier_type> quantifier,
            std::optional<corresponding_type> corresponding,
            std::unique_ptr<expression> right,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param left the left term
     * @param operator_kind the binary operator
     * @param quantifier the set quantifier
     * @param corresponding the corresponding clause
     * @param right the right term
     * @param region the node region
     * @attention this will take copy of arguments
     */
    explicit binary_expression(
            expression&& left,
            operator_kind_type operator_kind,
            std::optional<quantifier_type> quantifier,
            common::rvalue_list<name::simple> corresponding,
            expression&& right,
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param left the left term
     * @param operator_kind the binary operator
     * @param quantifier the set quantifier
     * @param right the right term
     * @param region the node region
     * @attention this will take copy of arguments
     */
    explicit binary_expression(
            expression&& left,
            operator_kind_type operator_kind,
            std::optional<quantifier_type> quantifier,
            expression&& right,
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param left the left term
     * @param operator_kind the binary operator
     * @param right the right term
     * @param region the node region
     * @attention this will take copy of arguments
     */
    explicit binary_expression(
            expression&& left,
            operator_kind_type operator_kind,
            expression&& right,
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit binary_expression(::takatori::util::clone_tag_t, binary_expression const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit binary_expression(::takatori::util::clone_tag_t, binary_expression&& other);

    [[nodiscard]] binary_expression* clone() const& override;
    [[nodiscard]] binary_expression* clone() && override;

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
    [[nodiscard]] std::unique_ptr<expression>& left() noexcept;

    /// @copydoc left()
    [[nodiscard]] std::unique_ptr<expression> const& left() const noexcept;

    /**
     * @brief returns the right term.
     * @return the right term
     */
    [[nodiscard]] std::unique_ptr<expression>& right() noexcept;

    /// @copydoc right()
    [[nodiscard]] std::unique_ptr<expression> const& right() const noexcept;

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
    std::unique_ptr<expression> left_;
    operator_kind_type operator_kind_;
    std::optional<quantifier_type> quantifier_;
    std::optional<corresponding_type> corresponding_;
    std::unique_ptr<expression> right_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, binary_expression const& value);

} // namespace mizugaki::ast::query
