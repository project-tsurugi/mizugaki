#pragma once

#include <takatori/util/clone_tag.h>

#include <mizugaki/ast/element.h>
#include <mizugaki/ast/common/clone_wrapper.h>
#include <mizugaki/ast/common/rvalue_list.h>
#include <mizugaki/ast/common/vector.h>
#include <mizugaki/ast/name/simple.h>

#include "expression.h"

namespace mizugaki::ast::query {

/**
 * @brief declares a query name.
 * @note `7.12 <query expression>` - `<with list element>`
 */
class with_element : public element {
public:
    /**
     * @brief creates a new invalid instance.
     * @note this is only used in parser generator.
     */
    explicit with_element() = default;

    /**
     * @brief creates a new instance.
     * @param name the query name
     * @param column_names the correlation column names
     * @param expression the query expression
     * @param region the element region
     */
    explicit with_element(
            std::unique_ptr<name::simple> name,
            std::vector<std::unique_ptr<name::simple>> column_names,
            std::unique_ptr<ast::query::expression> expression,
            // FIXME: search clause
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param name the query name
     * @param expression the query expression
     * @param region the element region
     */
    with_element(
            name::simple&& name,
            ast::query::expression&& expression,
            // FIXME: search clause
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param name the query name
     * @param column_names the correlation column names
     * @param expression the query expression
     * @param region the element region
     */
    with_element(
            name::simple&& name,
            common::rvalue_list<name::simple> column_names,
            ast::query::expression&& expression,
            // FIXME: search clause
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit with_element(::takatori::util::clone_tag_t, with_element const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit with_element(::takatori::util::clone_tag_t, with_element&& other);

    /**
     * @brief returns the correlation name.
     * @return the correlation name
     */
    [[nodiscard]] std::unique_ptr<name::simple>& name() noexcept;

    /// @copydoc name()
    [[nodiscard]] std::unique_ptr<name::simple> const& name() const noexcept;

    /**
     * @brief returns the query expression to be named.
     * @return the query expression
     */
    [[nodiscard]] std::unique_ptr<class expression>& expression() noexcept;

    /// @copydoc expression()
    [[nodiscard]] std::unique_ptr<class expression> const& expression() const noexcept;

    /**
     * @brief returns the correlation column names.
     * @return the column names
     * @return empty if they are not defined explicitly
     */
    [[nodiscard]] std::vector<std::unique_ptr<name::simple>>& column_names() noexcept;

    /// @copydoc column_names()
    [[nodiscard]] std::vector<std::unique_ptr<name::simple>> const& column_names() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(with_element const& a, with_element const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(with_element const& a, with_element const& b) noexcept;

    /**
     * @brief dumps structure information of the given value into the target acceptor.
     * @param acceptor the target acceptor
     * @param value the target value
     * @return the output
     */
    friend ::takatori::serializer::object_acceptor& operator<<(
            ::takatori::serializer::object_acceptor& acceptor,
            with_element const& value);

private:
    common::clone_wrapper<std::unique_ptr<name::simple>> name_ {};
    common::clone_wrapper<std::vector<std::unique_ptr<name::simple>>> column_names_ {};
    common::clone_wrapper<std::unique_ptr<class expression>> expression_ {};
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, with_element const& value);

} // namespace mizugaki::ast::query
