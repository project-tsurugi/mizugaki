#pragma once

#include <optional>

#include <takatori/util/clone_tag.h>

#include <mizugaki/ast/common/regioned.h>
#include <mizugaki/ast/name/name.h>

#include "expression.h"

namespace mizugaki::ast::query {

/**
 * @brief refers a table by its name (`TABLE`).
 * @note `7.12 <query expression>` - `<explicit table>`
 */
class table_reference final : public expression {

    using super = expression;

public:
    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::table_reference;

    /**
     * @brief creates a new instance.
     * @param name the table or query name
     * @param region the node region
     */
    explicit table_reference(
            std::unique_ptr<name::name> name,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param name the table or query name
     * @param region the node region
     * @attention this may take a copy of argument
     */
    explicit table_reference(
            name::name&& name,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit table_reference(::takatori::util::clone_tag_t, table_reference const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit table_reference(::takatori::util::clone_tag_t, table_reference&& other);

    [[nodiscard]] table_reference* clone() const& override;
    [[nodiscard]] table_reference* clone() && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the table name.
     * @return the table name
     */
    [[nodiscard]] std::unique_ptr<name::name>& name() noexcept;

    /// @brief name()
    [[nodiscard]] std::unique_ptr<name::name> const& name() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(table_reference const& a, table_reference const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(table_reference const& a, table_reference const& b) noexcept;

protected:
    [[nodiscard]] bool equals(expression const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    std::unique_ptr<name::name> name_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, table_reference const& value);

} // namespace mizugaki::ast::query
