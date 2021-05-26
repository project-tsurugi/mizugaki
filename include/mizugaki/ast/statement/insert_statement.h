#pragma once

#include <optional>

#include <takatori/util/clone_tag.h>
#include <takatori/util/rvalue_ptr.h>

#include <mizugaki/ast/common/vector.h>
#include <mizugaki/ast/common/rvalue_list.h>
#include <mizugaki/ast/name/name.h>
#include <mizugaki/ast/name/simple.h>
#include <mizugaki/ast/query/expression.h>

#include "statement.h"

namespace mizugaki::ast::statement {

/**
 * @brief creates new rows in a table.
 * @note `14.8 <insert statement>`
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
            std::unique_ptr<name::name> table_name,
            std::vector<std::unique_ptr<name::simple>> columns,
            // FIXME: overriding clause
            std::unique_ptr<query::expression> expression,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param table_name the target table name
     * @param columns the target columns
     * @param expression the source table expression, or empty if insert a row with default values
     * @param region the node region
     * @attention this will take copy of arguments
     */
    explicit insert_statement(
            name::name&& table_name,
            common::rvalue_list<name::simple> columns,
            ::takatori::util::rvalue_ptr<query::expression> expression,
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit insert_statement(::takatori::util::clone_tag_t, insert_statement const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit insert_statement(::takatori::util::clone_tag_t, insert_statement&& other);

    [[nodiscard]] insert_statement* clone() const& override;
    [[nodiscard]] insert_statement* clone() && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the target table name.
     * @return the target table name
     */
    [[nodiscard]] std::unique_ptr<name::name>& table_name() noexcept;

    /// @brief table_name()
    [[nodiscard]] std::unique_ptr<name::name> const& table_name() const noexcept;

    /**
     * @brief returns the target columns.
     * @return the target columns
     * @return empty if the target columns are omitted
     */
    [[nodiscard]] std::vector<std::unique_ptr<name::simple>>& columns() noexcept;

    /// @copydoc columns()
    [[nodiscard]] std::vector<std::unique_ptr<name::simple>> const& columns() const noexcept;

    /**
     * @brief returns the query expression.
     * @return the query expression
     * @return empty if `DEFAULT VALUES` is specified
     */
    [[nodiscard]] std::unique_ptr<query::expression>& expression() noexcept;

    /// @brief expression()
    [[nodiscard]] std::unique_ptr<query::expression> const& expression() const noexcept;
    
    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(insert_statement const& a, insert_statement const& b) noexcept;
    
    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(insert_statement const& a, insert_statement const& b) noexcept;
    

protected:
    [[nodiscard]] bool equals(statement const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    std::unique_ptr<name::name> table_name_;
    std::vector<std::unique_ptr<name::simple>> columns_;
    std::unique_ptr<query::expression> expression_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, insert_statement const& value);

} // namespace mizugaki::ast::statement
