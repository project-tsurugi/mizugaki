#pragma once

#include <memory>

#include <takatori/util/clone_tag.h>

#include <mizugaki/ast/common/regioned.h>
#include <mizugaki/ast/scalar/expression.h>

#include "constraint.h"

namespace mizugaki::ast::statement {

/**
 * @brief constraint with a scalar expression.
 * @note `11.4 <column definition>` - `<default clause>`
 * @note `11.6 <table constraint definition>`
 */
class expression_constraint final : public constraint {

    using super = constraint;

public:
    /// @brief the constraint kind type.
    using constraint_kind_type = common::regioned<node_kind_type>;

    /// @brief the available node kind of this.
    static constexpr constraint_kind_set tags {
            node_kind_type::check,
            node_kind_type::default_clause,
            node_kind_type::generation_clause,
    };

    /**
     * @brief creates a new instance.
     * @param constraint_kind the constraint kind
     * @param expression the constraint expression
     * @param region the node region
     */
    explicit expression_constraint(
            constraint_kind_type constraint_kind,
            std::unique_ptr<scalar::expression> expression,
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param constraint_kind the constraint kind
     * @param expression the constraint expression
     * @param region the node region
     * @attention this will take copy of arguments
     */
    explicit expression_constraint(
            constraint_kind_type constraint_kind,
            scalar::expression&& expression,
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit expression_constraint(::takatori::util::clone_tag_t, expression_constraint const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit expression_constraint(::takatori::util::clone_tag_t, expression_constraint&& other);

    [[nodiscard]] expression_constraint* clone() const& override;
    [[nodiscard]] expression_constraint* clone() && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the constraint kind.
     * @return the constraint kind
     */
    [[nodiscard]] constraint_kind_type& constraint_kind() noexcept;

    /// @copydoc constraint_kind()
    [[nodiscard]] constraint_kind_type const& constraint_kind() const noexcept;

    /**
     * @brief returns the constraint expression.
     * @return the constraint expression
     */
    [[nodiscard]] std::unique_ptr<scalar::expression>& expression() noexcept;

    /// @copydoc expression()
    [[nodiscard]] std::unique_ptr<scalar::expression> const& expression() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(expression_constraint const& a, expression_constraint const& b) noexcept;
    
    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(expression_constraint const& a, expression_constraint const& b) noexcept;

protected:
    [[nodiscard]] bool equals(constraint const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    constraint_kind_type constraint_kind_;
    std::unique_ptr<scalar::expression> expression_ {};
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, expression_constraint const& value);

} // namespace mizugaki::ast::statement
