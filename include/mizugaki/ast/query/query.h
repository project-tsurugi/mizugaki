#pragma once

#include <optional>

#include <takatori/util/object_creator.h>
#include <takatori/util/rvalue_ptr.h>

#include <mizugaki/ast/common/regioned.h>
#include <mizugaki/ast/common/rvalue_list.h>
#include <mizugaki/ast/common/vector.h>
#include <mizugaki/ast/scalar/expression.h>
#include <mizugaki/ast/scalar/set_quantifier.h>
#include <mizugaki/ast/table/expression.h>

#include "expression.h"
#include "select_element.h"
#include "group_by_clause.h"
#include "mizugaki/ast/common/sort_element.h"

namespace mizugaki::ast::query {

using scalar::set_quantifier;

/**
 * @brief processes table expressions (`SELECT`).
 * @note `7.11 <query specification>`
 */
class query final : public expression {

    using super = expression;

public:
    /// @brief the quantifier type.
    using quantifier_type = common::regioned<set_quantifier>;
    
    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::query;

    /**
     * @brief creates a new instance.
     * @param quantifier the set quantifier
     * @param elements the select elements
     * @param from the from clause
     * @param where the where clause
     * @param group_by the group by clause
     * @param having the having clause
     * @param order_by the order by clause
     * @param limit the limit clause
     * @param region the node region
     */
    explicit query(
            std::optional<quantifier_type> quantifier,
            common::vector<::takatori::util::unique_object_ptr<select_element>> elements,
            common::vector<::takatori::util::unique_object_ptr<table::expression>> from,
            ::takatori::util::unique_object_ptr<scalar::expression> where = {},
            std::optional<group_by_clause> group_by = {},
            ::takatori::util::unique_object_ptr<scalar::expression> having = {},
            common::vector<common::sort_element> order_by = {},
            ::takatori::util::unique_object_ptr<scalar::expression> limit = {},
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param elements the select elements
     * @param from the from clause
     * @param where the where clause
     * @param group_by the group by clause
     * @param having the having clause
     * @param order_by the order by clause
     * @param limit the limit clause
     * @param region the node region
     * @attention this may take copy of elements
     */
    query(
            common::rvalue_list<select_element> elements,
            common::rvalue_list<table::expression> from,
            ::takatori::util::rvalue_ptr<scalar::expression> where = {},
            std::optional<group_by_clause> group_by = {},
            ::takatori::util::rvalue_ptr<scalar::expression> having = {},
            std::initializer_list<common::sort_element> order_by = {},
            ::takatori::util::rvalue_ptr<scalar::expression> limit = {},
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param quantifier the set quantifier
     * @param elements the select elements
     * @param from the from clause
     * @param where the where clause
     * @param group_by the group by clause
     * @param having the having clause
     * @param order_by the order by clause
     * @param limit the limit clause
     * @param region the node region
     * @attention this may take copy of elements
     */
    query(
            std::optional<quantifier_type> quantifier,
            common::rvalue_list<select_element> elements,
            common::rvalue_list<table::expression> from,
            ::takatori::util::rvalue_ptr<scalar::expression> where = {},
            std::optional<group_by_clause> group_by = {},
            ::takatori::util::rvalue_ptr<scalar::expression> having = {},
            std::initializer_list<common::sort_element> order_by = {},
            ::takatori::util::rvalue_ptr<scalar::expression> limit = {},
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit query(query const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit query(query&& other, ::takatori::util::object_creator creator);

    [[nodiscard]] query* clone(::takatori::util::object_creator creator) const& override;
    [[nodiscard]] query* clone(::takatori::util::object_creator creator) && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the select elements.
     * @return the select elements
     */
    [[nodiscard]] common::vector<::takatori::util::unique_object_ptr<select_element>>& elements() noexcept;

    /// @copydoc elements()
    [[nodiscard]] common::vector<::takatori::util::unique_object_ptr<select_element>> const& elements() const noexcept;

    /**
     * @brief returns the `FROM` clause.
     * @return the `FROM` clause
     * @note `7.5 <from clause>`
     */
    [[nodiscard]] common::vector<::takatori::util::unique_object_ptr<table::expression>>& from() noexcept;

    /// @copydoc from()
    [[nodiscard]] common::vector<::takatori::util::unique_object_ptr<table::expression>> const& from() const noexcept;

    /**
     * @brief returns the `WHERE` clause.
     * @return the `WHERE` clause
     * @return empty if it is not declared
     * @note `7.8 <where clause>`
     */
    [[nodiscard]] ::takatori::util::unique_object_ptr<scalar::expression>& where() noexcept;

    /// @copydoc where()
    [[nodiscard]] ::takatori::util::unique_object_ptr<scalar::expression> const& where() const noexcept;

    /**
     * @brief returns the `GROUP BY` clause.
     * @return the `GROUP BY` clause
     * @return empty if it is not declared
     * @note `7.9 <group by clause>`
     */
    [[nodiscard]] std::optional<group_by_clause>& group_by() noexcept;

    /// @copydoc group_by()
    [[nodiscard]] std::optional<group_by_clause> const& group_by() const noexcept;

    /**
     * @brief returns the `HAVING` clause.
     * @return the `HAVING` clause
     * @return empty if it is not declared
     * @note `7.10 <having clause>`
     */
    [[nodiscard]] ::takatori::util::unique_object_ptr<scalar::expression>& having() noexcept;

    /// @copydoc having()
    [[nodiscard]] ::takatori::util::unique_object_ptr<scalar::expression> const& having() const noexcept;

    /**
     * @brief returns the `ORDER BY` clause.
     * @return the `ORDER BY` clause
     * @return empty if it is not declared
     * @note `14.1 <declare cursor>` - `<order by clause>`
     */
    [[nodiscard]] common::vector<common::sort_element>& order_by() noexcept;

    /// @copydoc order_by()
    [[nodiscard]] common::vector<common::sort_element> const& order_by() const noexcept;

    /**
     * @brief returns the `LIMIT` clause.
     * @return the `LIMIT` clause
     * @return empty if it is not declared
     * @note this is a common extension for SQL.
     */
    [[nodiscard]] ::takatori::util::unique_object_ptr<scalar::expression>& limit() noexcept;

    /// @copydoc limit()
    [[nodiscard]] ::takatori::util::unique_object_ptr<scalar::expression> const& limit() const noexcept;

    /**
     * @brief returns the set quantifier.
     * @return the set quantifier
     * @return empty if it is not declared
     */
    [[nodiscard]] std::optional<quantifier_type>& quantifier() noexcept;

    /// @copydoc quantifier()
    [[nodiscard]] std::optional<quantifier_type> const& quantifier() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(query const& a, query const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(query const& a, query const& b) noexcept;

protected:
    [[nodiscard]] bool equals(expression const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    std::optional<quantifier_type> quantifier_;
    common::vector<::takatori::util::unique_object_ptr<select_element>> elements_;
    common::vector<::takatori::util::unique_object_ptr<table::expression>> from_;
    ::takatori::util::unique_object_ptr<scalar::expression> where_;
    std::optional<group_by_clause> group_by_;
    ::takatori::util::unique_object_ptr<scalar::expression> having_;
    common::vector<common::sort_element> order_by_;
    ::takatori::util::unique_object_ptr<scalar::expression> limit_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, query const& value);

} // namespace mizugaki::ast::query
