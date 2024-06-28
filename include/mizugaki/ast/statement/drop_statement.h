#pragma once

#include <memory>
#include <optional>
#include <vector>

#include <takatori/util/clone_tag.h>

#include <mizugaki/ast/common/regioned.h>
#include <mizugaki/ast/name/name.h>
#include <mizugaki/ast/name/simple.h>
#include <mizugaki/ast/query/expression.h>

#include "statement.h"
#include "drop_statement_option.h"

namespace mizugaki::ast::statement {

/**
 * @brief removes definitions.
 * @note `11.20 <drop table statement>`
 * @note `11.22 <drop view statement>`
 * @note `11.2 <drop schema statement>`
 */
class drop_statement final : public statement {

    using super = statement;

public:
    /// @brief the statement kind type.
    using statement_kind_type = common::regioned<node_kind_type>;

    /// @brief option type with element region information.
    using option_type = common::regioned<drop_statement_option>;

    /// @brief the available node kind of this.
    static constexpr kind_set tags {
            node_kind_type::drop_table_statement,
            node_kind_type::drop_index_statement,
            node_kind_type::drop_view_statement,
            node_kind_type::drop_sequence_statement,
            node_kind_type::drop_schema_statement,
    };

    /**
     * @brief creates a new instance.
     * @param statement_kind the statement kind
     * @param name the target element name
     * @param options the declared options
     * @param region the node region
     */
    explicit drop_statement(
            statement_kind_type statement_kind,
            std::unique_ptr<name::name> name,
            std::vector<option_type> options = {},
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param statement_kind the statement kind
     * @param name the target element name
     * @param options the declared options
     * @param region the node region
     * @attention this will take copy of arguments
     */
    explicit drop_statement(
            statement_kind_type statement_kind,
            name::name&& name,
            std::initializer_list<option_type> options = {},
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit drop_statement(::takatori::util::clone_tag_t, drop_statement const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit drop_statement(::takatori::util::clone_tag_t, drop_statement&& other);

    [[nodiscard]] drop_statement* clone() const& override;
    [[nodiscard]] drop_statement* clone() && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the statement kind.
     * @return the statement kind
     */
    [[nodiscard]] statement_kind_type& statement_kind() noexcept;

    /// @copydoc statement_kind()
    [[nodiscard]] statement_kind_type const& statement_kind() const noexcept;

    /**
     * @brief returns the target element name.
     * @return the target element name
     */
    [[nodiscard]] std::unique_ptr<name::name>& name() noexcept;

    /// @brief name()
    [[nodiscard]] std::unique_ptr<name::name> const& name() const noexcept;

    /**
     * @brief returns list of declared options.
     * @return declared options
     */
    [[nodiscard]] std::vector<option_type>& options() noexcept;

    /// @copydoc options()
    [[nodiscard]] std::vector<option_type> const& options() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(drop_statement const& a, drop_statement const& b) noexcept;
    
    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(drop_statement const& a, drop_statement const& b) noexcept;

protected:
    [[nodiscard]] bool equals(statement const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    statement_kind_type statement_kind_;
    std::unique_ptr<name::name> name_;
    std::vector<option_type> options_ {};
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, drop_statement const& value);

} // namespace mizugaki::ast::statement
