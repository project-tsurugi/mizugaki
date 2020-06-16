#pragma once

#include <takatori/util/object_creator.h>

#include <mizugaki/ast/common/regioned.h>
#include <mizugaki/ast/query/expression.h>

#include "expression.h"

namespace mizugaki::ast::scalar {

/**
 * @brief processes sub-query.
 * @details This may represent scalar, row, or table subquery (depends on its context).
 * @note `7.14 <subquery>`
 */
class subquery final : public expression {

    using super = expression;

public:
    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::subquery;

    /**
     * @brief creates a new instance.
     * @param expression the query expression
     * @param region the node region
     */
    explicit subquery(
            ::takatori::util::unique_object_ptr<query::expression> expression,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit subquery(subquery const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit subquery(subquery&& other, ::takatori::util::object_creator creator);

    [[nodiscard]] subquery* clone(::takatori::util::object_creator creator) const& override;
    [[nodiscard]] subquery* clone(::takatori::util::object_creator creator) && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the query expression.
     * @return the query expression
     */
    [[nodiscard]] ::takatori::util::unique_object_ptr<query::expression>& expression() noexcept;

    /// @copydoc expression()
    [[nodiscard]] ::takatori::util::unique_object_ptr<query::expression> const& expression() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(subquery const& a, subquery const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(subquery const& a, subquery const& b) noexcept;

protected:
    [[nodiscard]] bool equals(scalar::expression const& other) const noexcept override;

private:
    ::takatori::util::unique_object_ptr<query::expression> expression_;
};

} // namespace mizugaki::ast::scalar
