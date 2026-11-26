#pragma once

#include <memory>

#include <takatori/util/clone_tag.h>
#include <takatori/util/rvalue_ptr.h>

#include <mizugaki/ast/element.h>
#include <mizugaki/ast/common/clone_wrapper.h>
#include <mizugaki/ast/name/name.h>
#include <mizugaki/ast/name/simple.h>
#include <mizugaki/ast/scalar/expression.h>

namespace mizugaki::ast::statement {

/**
 * @brief an element of partition key in partition definitions.
 * @details This represents either a column reference, or an expression with its name (virtual column).
 * @see partition_definition
 */
class partition_key_element final : public element {
public:
    /**
     * @brief creates a new empty instance.
     * @note this is used in parser generator.
     */
    explicit partition_key_element() = default;

    /**
     * @brief creates a new instance.
     * @param value the key column, or the expression of the virtual column
     * @param name the virtual column name, or empty if this is not a virtual column
     * @param region the element region
     */
    explicit partition_key_element(
            std::unique_ptr<scalar::expression> value,
            std::unique_ptr<name::simple> name = {},
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param value the key column, or the expression of the virtual column
     * @param name the virtual column name, or empty if this is not a virtual column
     * @param region the element region
     * @attention this will take copy of arguments
     */
    partition_key_element( // NOLINT: DSL style constructor
            scalar::expression&& value,
            ::takatori::util::rvalue_ptr<name::simple> name = {},
            region_type region = {});

    /**
     * @brief creates a new instance which refers to the given column.
     * @param column the target column name
     * @param region the element region
     * @attention this will take copy of arguments
     */
    partition_key_element( // NOLINT: DSL style constructor
            name::name&& column,
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit partition_key_element(::takatori::util::clone_tag_t, partition_key_element const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit partition_key_element(::takatori::util::clone_tag_t, partition_key_element&& other);

    /**
     * @brief returns the key column, or the expression of the virtual column.
     * @return the key column, or the expression of the virtual column
     */
    [[nodiscard]] std::unique_ptr<scalar::expression>& value() noexcept;

    /// @copydoc value()
    [[nodiscard]] std::unique_ptr<scalar::expression> const& value() const noexcept;

    /**
     * @brief returns the virtual column name.
     * @return the virtual column name
     * @return empty if this is not a virtual column
     */
    [[nodiscard]] std::unique_ptr<name::simple>& name() noexcept;

    /// @copydoc name()
    [[nodiscard]] std::unique_ptr<name::simple> const& name() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(partition_key_element const& a, partition_key_element const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(partition_key_element const& a, partition_key_element const& b) noexcept;

    /**
     * @brief dumps structure information of the given value into the target acceptor.
     * @param acceptor the target acceptor
     * @param value the target value
     * @return the output
     */
    friend ::takatori::serializer::object_acceptor& operator<<(
            ::takatori::serializer::object_acceptor& acceptor,
            partition_key_element const& value);

private:
    common::clone_wrapper<std::unique_ptr<scalar::expression>> value_ {};
    common::clone_wrapper<std::unique_ptr<name::simple>> name_ {};
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, partition_key_element const& value);

} // namespace mizugaki::ast::statement
