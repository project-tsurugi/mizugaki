#pragma once

#include <optional>

#include <takatori/util/clone_tag.h>
#include <takatori/util/rvalue_ptr.h>

#include <mizugaki/ast/common/vector.h>
#include <mizugaki/ast/name/name.h>
#include <mizugaki/ast/scalar/expression.h>

#include "statement.h"
#include "set_element.h"

namespace mizugaki::ast::statement {

/**
 * @brief update rows on a table.
 * @note `14.9 <update statement: positioned>`
 * @note `14.10 <update statement: searched>`
 */
class update_statement final : public statement {

    using super = statement;

public:
    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::update_statement;

    /**
     * @brief creates a new instance.
     * @param table_name the target table name
     * @param elements individual update elements
     * @param where expression the where clause, may be `CURRENT OF cursor_name`
     * @param region the node region
     */
    explicit update_statement(
            std::unique_ptr<name::name> table_name,
            std::vector<set_element> elements,
            std::unique_ptr<scalar::expression> where,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param table_name the target table name
     * @param elements individual update elements
     * @param where expression the where clause, may be `CURRENT OF cursor_name`
     * @param region the node region
     * @attention this will take copy of arguments
     */
    explicit update_statement(
            name::name&& table_name,
            std::initializer_list<set_element> elements,
            ::takatori::util::rvalue_ptr<scalar::expression> where = {},
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit update_statement(::takatori::util::clone_tag_t, update_statement const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit update_statement(::takatori::util::clone_tag_t, update_statement&& other);

    [[nodiscard]] update_statement* clone() const& override;
    [[nodiscard]] update_statement* clone() && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the target table name.
     * @return the target table name
     */
    [[nodiscard]] std::unique_ptr<name::name>& table_name() noexcept;

    /// @brief table_name()
    [[nodiscard]] std::unique_ptr<name::name> const& table_name() const noexcept;

    /**
     * @brief returns the individual update elements.
     * @return the update elements
     */
    [[nodiscard]] std::vector<set_element>& elements() noexcept {
        return elements_;
    }

    /// @copydoc elements()
    [[nodiscard]] std::vector<set_element> const& elements() const noexcept {
        return elements_;
    }

    /**
     * @brief returns the search condition expression.
     * @return the search condition
     * @return FIXME: `CURRENT OF cursor_name`
     * @return empty if this statement has no search condition
     */
    [[nodiscard]] std::unique_ptr<scalar::expression>& where() noexcept;

    /// @brief expression()
    [[nodiscard]] std::unique_ptr<scalar::expression> const& where() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(update_statement const& a, update_statement const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(update_statement const& a, update_statement const& b) noexcept;


protected:
    [[nodiscard]] bool equals(statement const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    std::unique_ptr<name::name> table_name_;
    std::vector<set_element> elements_;
    std::unique_ptr<scalar::expression> where_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, update_statement const& value);

} // namespace mizugaki::ast::statement
