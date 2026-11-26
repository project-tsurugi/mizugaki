#pragma once

#include <memory>
#include <vector>

#include <takatori/util/clone_tag.h>

#include <mizugaki/ast/element.h>
#include <mizugaki/ast/common/clone_wrapper.h>
#include <mizugaki/ast/common/rvalue_list.h>
#include <mizugaki/ast/scalar/comparison_predicate.h>

namespace mizugaki::ast::statement {

/**
 * @brief represents a criteria of target partitions (`PARTITION (...)`).
 * @details This criteria satisfies all the predicates, and each predicate represents a comparison between
 *      a partition key column and a literal.
 *      This element does not normalize the predicates, that is, the column may appear in either side of the
 *      comparison, and the predicates are ordered as they appear in the source text.
 * @note This syntax element is out of SQL standard.
 * @see drop_partition_action
 */
class partition_criteria final : public element {
public:
    /// @brief the predicate type.
    using predicate_type = scalar::comparison_predicate;

    /**
     * @brief creates a new empty instance.
     * @note this is used in parser generator.
     */
    explicit partition_criteria() = default;

    /**
     * @brief creates a new instance.
     * @param predicates the predicates
     * @param region the element region
     */
    explicit partition_criteria(
            std::vector<std::unique_ptr<predicate_type>> predicates,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param predicates the predicates
     * @param region the element region
     * @attention this will take copy of elements
     */
    partition_criteria( // NOLINT: DSL style constructor
            common::rvalue_list<predicate_type> predicates,
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit partition_criteria(::takatori::util::clone_tag_t, partition_criteria const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit partition_criteria(::takatori::util::clone_tag_t, partition_criteria&& other);

    /**
     * @brief returns the predicates.
     * @return the predicates
     */
    [[nodiscard]] std::vector<std::unique_ptr<predicate_type>>& predicates() noexcept;

    /// @copydoc predicates()
    [[nodiscard]] std::vector<std::unique_ptr<predicate_type>> const& predicates() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(partition_criteria const& a, partition_criteria const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(partition_criteria const& a, partition_criteria const& b) noexcept;

    /**
     * @brief dumps structure information of the given value into the target acceptor.
     * @param acceptor the target acceptor
     * @param value the target value
     * @return the output
     */
    friend ::takatori::serializer::object_acceptor& operator<<(
            ::takatori::serializer::object_acceptor& acceptor,
            partition_criteria const& value);

private:
    common::clone_wrapper<std::vector<std::unique_ptr<predicate_type>>> predicates_ {};
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, partition_criteria const& value);

} // namespace mizugaki::ast::statement
