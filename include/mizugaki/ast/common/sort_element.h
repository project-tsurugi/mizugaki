#pragma once

#include <optional>

#include <takatori/util/clone_tag.h>
#include <takatori/util/rvalue_ptr.h>

#include <mizugaki/ast/element.h>
#include <mizugaki/ast/common/regioned.h>
#include <mizugaki/ast/common/clone_wrapper.h>
#include <mizugaki/ast/common/ordering_specification.h>
#include <mizugaki/ast/common/null_ordering_specification.h>
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
    
    /// @brief the null location type.
    using null_location_type = regioned<null_ordering_specification>;

    /**
     * @brief creates a new empty instance.
     * @note this is used in parser generator.
     */
    explicit sort_element() = default;

    /**
     * @brief creates a new instance.
     * @param key the sort key
     * @param collation the optional collation name
     * @param direction the optional sort direction
     * @param null_location the optional nulls location
     * @param region the element region
     */
    explicit sort_element(
            std::unique_ptr<scalar::expression> key,
            std::unique_ptr<name::name> collation = {},
            std::optional<direction_type> direction = {},
            std::optional<null_location_type> null_location = {},
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param key the sort key
     * @param collation the optional collation name
     * @param direction the optional sort direction
     * @param null_location the optional nulls location
     * @param region the element region
     * @attention this will take copy of arguments
     */
    sort_element( // NOLINT: DSL style constructor
            scalar::expression&& key,
            ::takatori::util::rvalue_ptr<name::name> collation = {},
            std::optional<direction_type> direction = {},
            std::optional<null_location_type> null_location = {},
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param key the sort key
     * @param direction the optional sort direction
     * @param null_location the optional nulls location
     * @param region the element region
     * @attention this will take copy of arguments
     */
    sort_element(
            scalar::expression&& key,
            direction_type direction,
            std::optional<null_location_type> null_location = {},
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param key the sort key
     * @param collation the optional collation name
     * @param direction the optional sort direction
     * @param null_location the optional nulls location
     * @param region the element region
     * @attention this will take copy of arguments
     */
    sort_element( // NOLINT: DSL style constructor
            name::name&& key,
            ::takatori::util::rvalue_ptr<name::name> collation = {},
            std::optional<direction_type> direction = {},
            std::optional<null_location_type> null_location = {},
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param key the sort key
     * @param direction the optional sort direction
     * @param null_location the optional nulls location
     * @param region the element region
     * @attention this will take copy of arguments
     */
    sort_element(
            name::name&& key,
            direction_type direction,
            std::optional<null_location_type> null_location = {},
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit sort_element(::takatori::util::clone_tag_t, sort_element const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit sort_element(::takatori::util::clone_tag_t, sort_element&& other);

    /**
     * @brief returns the sort key expression.
     * @return the sort key expression
     */
    [[nodiscard]] std::unique_ptr<scalar::expression>& key() noexcept;

    /// @copydoc key()
    [[nodiscard]] std::unique_ptr<scalar::expression> const& key() const noexcept;

    /**
     * @brief returns the collation name.
     * @return the collation name
     * @return empty if it is not declared
     */
    [[nodiscard]] std::unique_ptr<name::name>& collation() noexcept;

    /// @copydoc collation()
    [[nodiscard]] std::unique_ptr<name::name> const& collation() const noexcept;

    /**
     * @brief returns the sort direction.
     * @return the sort direction
     * @return empty if it is not declared
     */
    [[nodiscard]] std::optional<direction_type>& direction() noexcept;

    /// @copydoc direction()
    [[nodiscard]] std::optional<direction_type> const& direction() const noexcept;

    /**
     * @brief returns the null location.
     * @return the sort null location
     * @return empty if it is not declared
     */
    [[nodiscard]] std::optional<null_location_type>& null_location() noexcept;

    /// @copydoc null_location()
    [[nodiscard]] std::optional<null_location_type> const& null_location() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(sort_element const& a, sort_element const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(sort_element const& a, sort_element const& b) noexcept;

    /**
     * @brief dumps structure information of the given value into the target acceptor.
     * @param acceptor the target acceptor
     * @param value the target value
     * @return the output
     */
    friend ::takatori::serializer::object_acceptor& operator<<(
            ::takatori::serializer::object_acceptor& acceptor,
            sort_element const& value);

private:
    common::clone_wrapper<std::unique_ptr<scalar::expression>> key_ {};
    common::clone_wrapper<std::unique_ptr<name::name>> collation_ {};
    std::optional<direction_type> direction_ {};
    std::optional<null_location_type> null_location_ {};
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, sort_element const& value);

} // namespace mizugaki::ast::common
