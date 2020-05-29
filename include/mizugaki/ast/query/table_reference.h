#pragma once

#include <optional>

#include <takatori/util/object_creator.h>

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
            ::takatori::util::unique_object_ptr<name::name> name,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit table_reference(table_reference const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit table_reference(table_reference&& other, ::takatori::util::object_creator creator);

    [[nodiscard]] table_reference* clone(::takatori::util::object_creator creator) const& override;
    [[nodiscard]] table_reference* clone(::takatori::util::object_creator creator) && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the table name.
     * @return the table name
     */
    [[nodiscard]] ::takatori::util::unique_object_ptr<name::name>& name() noexcept;

    /// @brief name()
    [[nodiscard]] ::takatori::util::unique_object_ptr<name::name> const& name() const noexcept;

private:
    ::takatori::util::unique_object_ptr<name::name> name_;
};

} // namespace mizugaki::ast::query
