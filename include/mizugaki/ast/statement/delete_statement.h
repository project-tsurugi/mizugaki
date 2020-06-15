#pragma once

#include <optional>

#include <takatori/util/object_creator.h>

#include <mizugaki/ast/name/name.h>
#include <mizugaki/ast/scalar/expression.h>

#include "statement.h"

namespace mizugaki::ast::statement {

/**
 * @brief deletes rows on a table.
 * @note `14.6 <delete statement: positioned>`
 * @note `14.7 <delete statement: searched>`
 */
class delete_statement final : public statement {

    using super = statement;

public:
    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::delete_statement;

    /**
     * @brief creates a new instance.
     * @param table_name the target table name
     * @param condition the optional search condition, may be `CURRENT OF cursor_name`
     * @param region the node region
     */
    explicit delete_statement(
            ::takatori::util::unique_object_ptr<name::name> table_name,
            ::takatori::util::unique_object_ptr<scalar::expression> condition,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit delete_statement(delete_statement const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit delete_statement(delete_statement&& other, ::takatori::util::object_creator creator);

    [[nodiscard]] delete_statement* clone(::takatori::util::object_creator creator) const& override;
    [[nodiscard]] delete_statement* clone(::takatori::util::object_creator creator) && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the target table name.
     * @return the target table name
     */
    [[nodiscard]] ::takatori::util::unique_object_ptr<name::name>& table_name() noexcept;

    /// @brief table_name()
    [[nodiscard]] ::takatori::util::unique_object_ptr<name::name> const& table_name() const noexcept;

    /**
     * @brief returns the search condition expression.
     * @return the search condition
     * @return FIXME: `CURRENT OF cursor_name`
     * @return empty if this statement has no search condition
     */
    [[nodiscard]] ::takatori::util::unique_object_ptr<scalar::expression>& condition() noexcept;

    /// @brief expression()
    [[nodiscard]] ::takatori::util::unique_object_ptr<scalar::expression> const& condition() const noexcept;

private:
    ::takatori::util::unique_object_ptr<name::name> table_name_;
    ::takatori::util::unique_object_ptr<scalar::expression> condition_;
};

} // namespace mizugaki::ast::statement
