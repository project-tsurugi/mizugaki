#pragma once

#include <optional>

#include <takatori/util/object_creator.h>
#include <takatori/util/rvalue_ptr.h>

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
     * @param where expression the where clause, may be `CURRENT OF cursor_name`
     * @param region the node region
     */
    explicit delete_statement(
            ::takatori::util::unique_object_ptr<name::name> table_name,
            ::takatori::util::unique_object_ptr<scalar::expression> where,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param table_name the target table name
     * @param where expression the where clause, may be `CURRENT OF cursor_name`
     * @param region the node region
     * @attention this will take copy of arguments
     */
    explicit delete_statement(
            name::name&& table_name,
            ::takatori::util::rvalue_ptr<scalar::expression> where = {},
            region_type region = {});

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
    [[nodiscard]] ::takatori::util::unique_object_ptr<scalar::expression>& where() noexcept;

    /// @brief expression()
    [[nodiscard]] ::takatori::util::unique_object_ptr<scalar::expression> const& where() const noexcept;
    
    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(delete_statement const& a, delete_statement const& b) noexcept;
    
    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(delete_statement const& a, delete_statement const& b) noexcept;
    

protected:
    [[nodiscard]] bool equals(statement const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    ::takatori::util::unique_object_ptr<name::name> table_name_;
    ::takatori::util::unique_object_ptr<scalar::expression> where_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, delete_statement const& value);

} // namespace mizugaki::ast::statement
