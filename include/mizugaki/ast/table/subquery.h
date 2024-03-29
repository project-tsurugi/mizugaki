#pragma once

#include <takatori/util/clone_tag.h>

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
     * @param is_lateral whether or not `LATERAL` is specified
     * @param expression the query expression
     * @param correlation the correlation declaration
     * @param region the node region
     */
    explicit subquery(
            bool_type is_lateral,
            std::unique_ptr<query::expression> expression,
            correlation_type correlation,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param expression the query expression
     * @param correlation the correlation declaration
     * @param is_lateral whether or not `LATERAL` is specified
     * @param region the node region
     * @attention this will take copy of arguments
     */
    explicit subquery(
            query::expression&& expression,
            correlation_type correlation,
            bool_type is_lateral = false,
            region_type region = {});

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
     * @brief returns the correlation declaration.
     * @return the correlation declaration
     * @return empty if there is no such the declaration
     */
    [[nodiscard]] correlation_type& correlation() noexcept;

    /// @copydoc correlation()
    [[nodiscard]] correlation_type const& correlation() const noexcept;

    /**
     * @brief returns whether or not `LATERAL` is specified.
     * @return true if `LATERAL` is specified
     * @return false
     */
    [[nodiscard]] bool_type& is_lateral() noexcept;

    /// @copydoc is_lateral()
    [[nodiscard]] bool_type const& is_lateral() const noexcept;

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
    [[nodiscard]] bool equals(table::expression const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    bool_type is_lateral_;
    std::unique_ptr<query::expression> expression_;
    correlation_type correlation_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, subquery const& value);

} // namespace mizugaki::ast::table
