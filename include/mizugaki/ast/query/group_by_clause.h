#pragma once

#include <takatori/util/clone_tag.h>

#include <mizugaki/ast/element.h>
#include <mizugaki/ast/common/clone_wrapper.h>
#include <mizugaki/ast/common/rvalue_list.h>
#include <mizugaki/ast/common/vector.h>
#include <mizugaki/ast/name/simple.h>

#include "grouping_element.h"

namespace mizugaki::ast::query {

/**
 * @brief specifies grouped table (`GROUP BY`).
 * @note `7.9 <group by clause>`
 */
class group_by_clause : public element {
public:
    /// @brief the element type.
    using element_type = grouping_element;

    /**
     * @brief creates a new instance.
     * @param elements the elements
     * @param region the element region
     */
    explicit group_by_clause(
            std::vector<std::unique_ptr<element_type>> elements,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param elements the elements
     * @param region the element region
     * @attention this will take copy of elements
     */
    group_by_clause(
            common::rvalue_list<element_type> elements,
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit group_by_clause(::takatori::util::clone_tag_t, group_by_clause const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit group_by_clause(::takatori::util::clone_tag_t, group_by_clause&& other);

    /**
     * @brief returns the grouping specification elements.
     * @return the grouping specification elements
     */
    [[nodiscard]] std::vector<std::unique_ptr<element_type>>& elements() noexcept;

    /// @copydoc elements()
    [[nodiscard]] std::vector<std::unique_ptr<element_type>> const& elements() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(group_by_clause const& a, group_by_clause const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(group_by_clause const& a, group_by_clause const& b) noexcept;

    /**
     * @brief dumps structure information of the given value into the target acceptor.
     * @param acceptor the target acceptor
     * @param value the target value
     * @return the output
     */
    friend ::takatori::serializer::object_acceptor& operator<<(
            ::takatori::serializer::object_acceptor& acceptor,
            group_by_clause const& value);

private:
    common::clone_wrapper<std::vector<std::unique_ptr<element_type>>> elements_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, group_by_clause const& value);

} // namespace mizugaki::ast::query
