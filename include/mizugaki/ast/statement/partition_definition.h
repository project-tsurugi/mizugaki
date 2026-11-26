#pragma once

#include <initializer_list>
#include <vector>

#include <takatori/util/clone_tag.h>

#include <mizugaki/ast/element.h>

#include "partition_key_element.h"

namespace mizugaki::ast::statement {

/**
 * @brief defines partitioning of a table (`PARTITION BY RANGE (...)`).
 * @details The partition key elements must be a prefix of the primary key columns of the table.
 *      Currently, this only represents range partitioning.
 * @see table_definition::partition()
 */
class partition_definition final : public element {
public:
    /// @brief the element type.
    using element_type = partition_key_element;

    /**
     * @brief creates a new instance.
     * @param elements the partition key elements
     * @param region the element region
     */
    explicit partition_definition(
            std::vector<element_type> elements,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param elements the partition key elements
     * @param region the element region
     * @attention this will take copy of elements
     */
    partition_definition( // NOLINT: DSL style constructor
            std::initializer_list<element_type> elements,
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit partition_definition(::takatori::util::clone_tag_t, partition_definition const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit partition_definition(::takatori::util::clone_tag_t, partition_definition&& other);

    /**
     * @brief returns the partition key elements.
     * @return the partition key elements
     */
    [[nodiscard]] std::vector<element_type>& elements() noexcept;

    /// @copydoc elements()
    [[nodiscard]] std::vector<element_type> const& elements() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(partition_definition const& a, partition_definition const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(partition_definition const& a, partition_definition const& b) noexcept;

    /**
     * @brief dumps structure information of the given value into the target acceptor.
     * @param acceptor the target acceptor
     * @param value the target value
     * @return the output
     */
    friend ::takatori::serializer::object_acceptor& operator<<(
            ::takatori::serializer::object_acceptor& acceptor,
            partition_definition const& value);

private:
    std::vector<element_type> elements_ {};
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, partition_definition const& value);

} // namespace mizugaki::ast::statement
