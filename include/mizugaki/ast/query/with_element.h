#pragma once

#include <takatori/util/object_creator.h>

#include <mizugaki/ast/element.h>
#include <mizugaki/ast/common/clone_wrapper.h>
#include <mizugaki/ast/common/vector.h>
#include <mizugaki/ast/name/simple.h>

#include "expression.h"

namespace mizugaki::ast::query {

/**
 * @brief declares a query name.
 * @note `7.12 <query expression>` - `<with list element>`
 */
class with_element : public element {
public:
    /**
     * @brief creates a new instance.
     * @param name the query name
     * @param expression the query expression
     * @param column_names the correlation column names
     * @param region the element region
     */
    explicit with_element(
            ::takatori::util::unique_object_ptr<name::simple> name,
            ::takatori::util::unique_object_ptr<class expression> expression,
            common::vector<::takatori::util::unique_object_ptr<name::simple>> column_names = {},
            // FIXME: search clause
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit with_element(with_element const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit with_element(with_element&& other, ::takatori::util::object_creator creator);

    /**
     * @brief returns the correlation name.
     * @return the correlation name
     */
    [[nodiscard]] ::takatori::util::unique_object_ptr<name::simple>& name() noexcept;

    /// @copydoc name()
    [[nodiscard]] ::takatori::util::unique_object_ptr<name::simple> const& name() const noexcept;

    /**
     * @brief returns the query expression to be named.
     * @return the query expression
     */
    [[nodiscard]] ::takatori::util::unique_object_ptr<class expression>& expression() noexcept;

    /// @copydoc expression()
    [[nodiscard]] ::takatori::util::unique_object_ptr<class expression> const& expression() const noexcept;

    /**
     * @brief returns the correlation column names.
     * @return the column names
     * @return empty if they are not defined explicitly
     */
    [[nodiscard]] common::vector<::takatori::util::unique_object_ptr<name::simple>>& column_names() noexcept;

    /// @copydoc column_names()
    [[nodiscard]] common::vector<::takatori::util::unique_object_ptr<name::simple>> const& column_names() const noexcept;

private:
    common::clone_wrapper<::takatori::util::unique_object_ptr<name::simple>> name_;
    common::clone_wrapper<::takatori::util::unique_object_ptr<class expression>> expression_;
    common::clone_wrapper<common::vector<::takatori::util::unique_object_ptr<name::simple>>> column_names_;
};

} // namespace mizugaki::ast::query
