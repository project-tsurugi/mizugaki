#pragma once

#include <takatori/util/clone_tag.h>

#include <mizugaki/ast/common/regioned.h>
#include <mizugaki/ast/query/expression.h>

#include "expression.h"

namespace mizugaki::ast::scalar {

/**
 * @brief processes sub-query.
 * @details This may represent scalar or row subquery (depends on its context).
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
            std::unique_ptr<query::expression> expression,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param expression the query expression
     * @param region the node region
     * @attention this will take a copy of argument
     */
    explicit subquery(
            query::expression&& expression,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit subquery(::takatori::util::clone_tag_t, subquery const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit subquery(::takatori::util::clone_tag_t, subquery&& other);

    [[nodiscard]] subquery* clone() const& override;
    [[nodiscard]] subquery* clone() && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the query expression.
     * @return the query expression
     */
    [[nodiscard]] std::unique_ptr<query::expression>& expression() noexcept;

    /// @copydoc expression()
    [[nodiscard]] std::unique_ptr<query::expression> const& expression() const noexcept;

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
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    std::unique_ptr<query::expression> expression_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, subquery const& value);

} // namespace mizugaki::ast::scalar
