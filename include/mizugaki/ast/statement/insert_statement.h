#pragma once

#include <optional>

#include <takatori/util/object_creator.h>

#include <mizugaki/ast/common/vector.h>
#include <mizugaki/ast/name/name.h>
#include <mizugaki/ast/name/simple.h>
#include <mizugaki/ast/query/expression.h>

#include "statement.h"

namespace mizugaki::ast::statement {

/**
 * @brief creates new rows in a table.
 * @note `14.8 <insert statement>`
 * @see insert_statement
 */
class insert_statement final : public statement {

    using super = statement;

public:
    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::insert_statement;

    /**
     * @brief creates a new instance.
     * @param table_name the target table name
     * @param columns the target columns
     * @param expression the source table expression, or empty if insert a row with default values
     * @param region the node region
     */
    explicit insert_statement(
            ::takatori::util::unique_object_ptr<name::name> table_name,
            common::vector<::takatori::util::unique_object_ptr<name::simple>> columns,
            // FIXME: overriding clause
            ::takatori::util::unique_object_ptr<query::expression> expression,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit insert_statement(insert_statement const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit insert_statement(insert_statement&& other, ::takatori::util::object_creator creator);

    [[nodiscard]] insert_statement* clone(::takatori::util::object_creator creator) const& override;
    [[nodiscard]] insert_statement* clone(::takatori::util::object_creator creator) && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the target table name.
     * @return the target table name
     */
    [[nodiscard]] ::takatori::util::unique_object_ptr<name::name>& table_name() noexcept;

    /// @brief table_name()
    [[nodiscard]] ::takatori::util::unique_object_ptr<name::name> const& table_name() const noexcept;

    /**
     * @brief returns the target columns.
     * @return the target columns
     * @return empty if the target columns are omitted
     */
    [[nodiscard]] common::vector<::takatori::util::unique_object_ptr<name::simple>>& columns() noexcept;

    /// @copydoc columns()
    [[nodiscard]] common::vector<::takatori::util::unique_object_ptr<name::simple>> const& columns() const noexcept;

    /**
     * @brief returns the query expression.
     * @return the query expression
     * @return empty if `DEFAULT VALUES` is specified
     */
    [[nodiscard]] ::takatori::util::unique_object_ptr<query::expression>& expression() noexcept;

    /// @brief expression()
    [[nodiscard]] ::takatori::util::unique_object_ptr<query::expression> const& expression() const noexcept;

private:
    ::takatori::util::unique_object_ptr<name::name> table_name_;
    common::vector<::takatori::util::unique_object_ptr<name::simple>> columns_;
    ::takatori::util::unique_object_ptr<query::expression> expression_;
};

} // namespace mizugaki::ast::statement
