#pragma once

#include <optional>

#include <takatori/util/object_creator.h>

#include <mizugaki/ast/element.h>
#include <mizugaki/ast/common/regioned.h>
#include <mizugaki/ast/common/clone_wrapper.h>
#include <mizugaki/ast/common/ordering_specification.h>
#include <mizugaki/ast/name/name.h>
#include <mizugaki/ast/scalar/expression.h>

namespace mizugaki::ast::common {

/**
 * @brief specifies sorted table (`ORDER BY`).
 * @note `14.1 <declare cursor>` - `<sort specification>`
 */
class sort_element : public element {
public:
    /// @brief the direction type.
    using direction_type = regioned<ordering_specification>;

    /**
     * @brief creates a new instance.
     * @param key the sort key
     * @param collation the optional collation name
     * @param direction the optional sort direction
     * @param region the element region
     */
    explicit sort_element(
            ::takatori::util::unique_object_ptr<scalar::expression> key,
            ::takatori::util::unique_object_ptr<name::name> collation = {},
            std::optional<direction_type> direction = {},
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit sort_element(sort_element const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit sort_element(sort_element&& other, ::takatori::util::object_creator creator);

    /**
     * @brief returns the sort key expression.
     * @return the sort key expression
     */
    [[nodiscard]] ::takatori::util::unique_object_ptr<scalar::expression>& key() noexcept;

    /// @copydoc key()
    [[nodiscard]] ::takatori::util::unique_object_ptr<scalar::expression> const& key() const noexcept;

    /**
     * @brief returns the collation name.
     * @return the collation name
     * @return empty if it is not declared
     */
    [[nodiscard]] ::takatori::util::unique_object_ptr<name::name>& collation() noexcept;

    /// @copydoc collation()
    [[nodiscard]] ::takatori::util::unique_object_ptr<name::name> const& collation() const noexcept;

    /**
     * @brief returns the sort direction.
     * @return the sort direction
     * @return empty if it is not declared
     */
    [[nodiscard]] std::optional<direction_type>& direction() noexcept;

    /// @copydoc direction()
    [[nodiscard]] std::optional<direction_type> const& direction() const noexcept;

private:
    common::clone_wrapper<::takatori::util::unique_object_ptr<scalar::expression>> key_;
    common::clone_wrapper<::takatori::util::unique_object_ptr<name::name>> collation_;
    std::optional<direction_type> direction_;
};

} // namespace mizugaki::ast::common