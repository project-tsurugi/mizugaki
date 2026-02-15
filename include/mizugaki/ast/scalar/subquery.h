#pragma once

#include <takatori/util/clone_tag.h>

#include <mizugaki/ast/query/expression.h>

#include "expression.h"
#include "expression_context_kind.h"

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

    /// @brief the sub-query kind.
    using context_kind_type = expression_context_kind;

    /**
     * @brief creates a new instance.
     * @param query the query expression
     * @param context_kind the sub-query kind
     * @param region the node region
     */
    explicit subquery(
            std::unique_ptr<ast::query::expression> query,
            context_kind_type context_kind = context_kind_type::scalar,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param query the query expression
     * @param context_kind the sub-query kind
     * @param region the node region
     * @attention this will take a copy of argument
     */
    explicit subquery(
            ast::query::expression&& query,
            context_kind_type context_kind = context_kind_type::scalar,
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
    [[nodiscard]] std::unique_ptr<ast::query::expression>& query() noexcept;

    /// @copydoc query()
    [[nodiscard]] std::unique_ptr<ast::query::expression> const& query() const noexcept;

    /**
     * @brief returns the sub-query kind.
     * @return the sub-query kind
     */
    [[nodiscard]] context_kind_type& context_kind() noexcept;

    /// @copydoc context_kind()
    [[nodiscard]] context_kind_type const& context_kind() const noexcept;

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
    std::unique_ptr<ast::query::expression> query_;
    context_kind_type context_kind_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, subquery const& value);

} // namespace mizugaki::ast::scalar
