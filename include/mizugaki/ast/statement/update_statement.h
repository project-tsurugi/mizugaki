#pragma once

#include <optional>

#include <takatori/util/object_creator.h>
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
            ::takatori::util::unique_object_ptr<name::name> table_name,
            common::vector<set_element> elements,
            ::takatori::util::unique_object_ptr<scalar::expression> where,
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
     * @param creator the object creator
     */
    explicit update_statement(update_statement const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit update_statement(update_statement&& other, ::takatori::util::object_creator creator);

    [[nodiscard]] update_statement* clone(::takatori::util::object_creator creator) const& override;
    [[nodiscard]] update_statement* clone(::takatori::util::object_creator creator) && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the target table name.
     * @return the target table name
     */
    [[nodiscard]] ::takatori::util::unique_object_ptr<name::name>& table_name() noexcept;

    /// @brief table_name()
    [[nodiscard]] ::takatori::util::unique_object_ptr<name::name> const& table_name() const noexcept;

    /**
     * @brief returns the individual update elements.
     * @return the update elements
     */
    [[nodiscard]] common::vector<set_element>& elements() noexcept {
        return elements_;
    }

    /// @copydoc elements()
    [[nodiscard]] common::vector<set_element> const& elements() const noexcept {
        return elements_;
    }

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
    ::takatori::util::unique_object_ptr<name::name> table_name_;
    common::vector<set_element> elements_;
    ::takatori::util::unique_object_ptr<scalar::expression> where_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, update_statement const& value);

} // namespace mizugaki::ast::statement
