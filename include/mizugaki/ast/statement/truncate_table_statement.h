#pragma once

#include <optional>

#include <takatori/util/clone_tag.h>

#include <mizugaki/ast/common/regioned.h>
#include <mizugaki/ast/name/name.h>

#include "statement.h"
#include "identity_column_restart_option.h"

namespace mizugaki::ast::statement {

/**
 * @brief truncates all rows in a table or cluster.
 * @note `[SQL-2011] 14.10 <truncate table statement>`
 */
class truncate_table_statement : public statement {

    using super = statement;

public:
    /// @brief the kind of this element.
    static constexpr node_kind_type tag = node_kind_type::truncate_table_statement;

    /// @brief option type for identity columns.
    using identity_column_option_type = common::regioned<identity_column_restart_option>;

    /**
     * @brief creates a new instance.
     * @param name the target table name
     * @param identity_column_option the option for identity columns
     * @param region the node region
     */
    explicit truncate_table_statement(
            std::unique_ptr<name::name> name,
            std::optional<identity_column_option_type> identity_column_option = {},
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param name the target table name
     * @param identity_column_option the option for identity columns
     * @param region the node region
     * @attention this will take copy of arguments
     */
    explicit truncate_table_statement(
            name::name&& name,
            std::optional<identity_column_option_type> identity_column_option = {},
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit truncate_table_statement(::takatori::util::clone_tag_t, truncate_table_statement const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit truncate_table_statement(::takatori::util::clone_tag_t, truncate_table_statement&& other);

    [[nodiscard]] truncate_table_statement* clone() const& override;
    [[nodiscard]] truncate_table_statement* clone() && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the table name.
     * @return the table name
     */
    [[nodiscard]] std::unique_ptr<name::name>& name() noexcept;

    /// @copydoc name()
    [[nodiscard]] std::unique_ptr<name::name> const& name() const noexcept;

    /**
     * @brief returns the identity column option for the truncating table.
     * @return the identity column option
     */
    [[nodiscard]] std::optional<identity_column_option_type>& identity_column_option() noexcept;

    /// @copydoc identity_column_option()
    [[nodiscard]] std::optional<identity_column_option_type> const& identity_column_option() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(truncate_table_statement const& a, truncate_table_statement const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(truncate_table_statement const& a, truncate_table_statement const& b) noexcept;

protected:
    [[nodiscard]] bool equals(statement const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    std::unique_ptr<name::name> name_ {};
    std::optional<identity_column_option_type> identity_column_option_ {};
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, truncate_table_statement const& value);

} // namespace mizugaki::ast::statement
