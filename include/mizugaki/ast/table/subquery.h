#pragma once

#include <takatori/util/object_creator.h>

#include <mizugaki/ast/common/regioned.h>
#include <mizugaki/ast/query/expression.h>

#include "expression.h"
#include "correlation_clause.h"

namespace mizugaki::ast::table {

/**
 * @brief processes table sub-query.
 * @note `7.6 <table reference>` - `<derived table>` and `<lateral derived table>`
 * @note `7.14 <subquery>`
 */
class subquery final : public expression {

    using super = expression;

public:
    /// @brief the correlation declaration type.
    using correlation_type = correlation_clause;

    /// @brief truth type with element region information.
    using bool_type = common::regioned<bool>;

    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::subquery;

    /**
     * @brief creates a new instance.
     * @param body the query expression
     * @param correlation the correlation declaration
     * @param is_lateral whether or not `LATERAL` is specified
     * @param region the node region
     */
    explicit subquery(
            ::takatori::util::unique_object_ptr<query::expression> body,
            correlation_type correlation,
            bool_type is_lateral = { false },
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
    [[nodiscard]] ::takatori::util::unique_object_ptr<query::expression>& body() noexcept;

    /// @copydoc body()
    [[nodiscard]] ::takatori::util::unique_object_ptr<query::expression> const& body() const noexcept;

    /// @copydoc body()
    [[nodiscard]] ::takatori::util::unique_object_ptr<query::expression>& operator*() noexcept;

    /// @copydoc body()
    [[nodiscard]] ::takatori::util::unique_object_ptr<query::expression> const& operator*() const noexcept;

    /**
     * @brief returns the correlation declaration.
     * @return the correlation declaration
     * @return empty if there is no such the declaration
     */
    [[nodiscard]] correlation_type& correlation() noexcept;

    /// @copydoc correlation()
    [[nodiscard]] correlation_type const& correlation() const noexcept;

    /**
     * @brief returns whether or not `LATERAL` is specified.
     * @return true if `ONLY` is specified
     * @return false
     */
    [[nodiscard]] bool_type& is_lateral() noexcept;

    /// @copydoc is_lateral()
    [[nodiscard]] bool_type const& is_lateral() const noexcept;

private:
    ::takatori::util::unique_object_ptr<query::expression> body_;
    correlation_type correlation_;
    bool_type is_lateral_;
};

} // namespace mizugaki::ast::table
