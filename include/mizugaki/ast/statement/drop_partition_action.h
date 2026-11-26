#pragma once

#include <initializer_list>
#include <vector>

#include <takatori/util/clone_tag.h>

#include <mizugaki/ast/node.h>

#include "alter_table_action.h"
#include "partition_criteria.h"

namespace mizugaki::ast::statement {

/**
 * @brief drop partition action of alter table statement (`DROP PARTITION (...), ...`).
 * @details This action drops the partitions which satisfy any of the target criteria.
 * @note This syntax element is out of SQL standard.
 */
class drop_partition_action final : public alter_table_action {

    using super = alter_table_action;

public:
    /// @brief the kind of this element.
    static constexpr alter_table_action_kind tag = alter_table_action_kind::drop_partition;

    /**
     * @brief creates a new instance.
     * @param target the target criteria
     * @param region the element region
     */
    explicit drop_partition_action(
            std::vector<partition_criteria> target,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param target the target criteria
     * @param region the element region
     * @attention this will take copy of arguments
     */
    drop_partition_action( // NOLINT: DSL style constructor
            std::initializer_list<partition_criteria> target,
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit drop_partition_action(::takatori::util::clone_tag_t, drop_partition_action const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit drop_partition_action(::takatori::util::clone_tag_t, drop_partition_action&& other);

    [[nodiscard]] drop_partition_action* clone() const& override;
    [[nodiscard]] drop_partition_action* clone() && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the target criteria.
     * @details The target partitions are the ones which satisfy any of the criteria.
     * @return the target criteria
     */
    [[nodiscard]] std::vector<partition_criteria>& target() noexcept;

    /// @copydoc target()
    [[nodiscard]] std::vector<partition_criteria> const& target() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(drop_partition_action const& a, drop_partition_action const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(drop_partition_action const& a, drop_partition_action const& b) noexcept;

protected:
    [[nodiscard]] bool equals(alter_table_action const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    std::vector<partition_criteria> target_ {};
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, drop_partition_action const& value);

} // namespace mizugaki::ast::statement
