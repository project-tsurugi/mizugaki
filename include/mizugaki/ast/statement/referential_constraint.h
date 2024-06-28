#pragma once

#include <memory>
#include <optional>
#include <vector>

#include <takatori/util/clone_tag.h>

#include <mizugaki/ast/common/regioned.h>
#include <mizugaki/ast/common/rvalue_list.h>
#include <mizugaki/ast/name/name.h>
#include <mizugaki/ast/name/simple.h>

#include "constraint.h"
#include "referential_action.h"

namespace mizugaki::ast::statement {

/**
 * @brief keyword only constraints.
 * @note `11.4 <column definition>` - `<column constraint>`
 */
class referential_constraint final : public constraint {

    using super = constraint;

public:
    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::referential;

    /// @brief the referential action type..
    using action_type = common::regioned<referential_action>;

    /**
     * @brief creates a new instance.
     * @param columns the source columns, or empty if they are not defined
     * @param target the reference target table name
     * @param target_columns the corresponding target columns, or empty if they are not defined
     * @param on_update referential action for update operations, or empty if it is not defined
     * @param on_delete referential action for delete operations, or empty if it is not defined
     * @param region the node region
     */
    explicit referential_constraint(
            std::vector<std::unique_ptr<name::simple>> columns,
            std::unique_ptr<name::name> target,
            std::vector<std::unique_ptr<name::simple>> target_columns = {},
            std::optional<action_type> on_update = {},
            std::optional<action_type> on_delete = {},
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param columns the source columns, or empty if they are not defined
     * @param target the reference target table name
     * @param target_columns the corresponding target columns, or empty if they are not defined
     * @param on_update referential action for update operations, or empty if it is not defined
     * @param on_delete referential action for delete operations, or empty if it is not defined
     * @param region the node region
     * @attention this will take copy of arguments
     */
    explicit referential_constraint(
            common::rvalue_list<name::simple> columns,
            name::name&& target,
            common::rvalue_list<name::simple> target_columns = {},
            std::optional<action_type> on_update = {},
            std::optional<action_type> on_delete = {},
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit referential_constraint(::takatori::util::clone_tag_t, referential_constraint const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit referential_constraint(::takatori::util::clone_tag_t, referential_constraint&& other);

    [[nodiscard]] referential_constraint* clone() const& override;
    [[nodiscard]] referential_constraint* clone() && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the source columns.
     * @return the source columns
     * @return empty if they are not declared (for column constraint)
     */
    [[nodiscard]] std::vector<std::unique_ptr<name::simple>>& columns() noexcept;

    /// @copydoc columns()
    [[nodiscard]] std::vector<std::unique_ptr<name::simple>> const& columns() const noexcept;
    
    /**
     * @brief returns the reference target table name.
     * @return the reference target table name
     */
    [[nodiscard]] std::unique_ptr<name::name>& target() noexcept;

    /// @copydoc target()
    [[nodiscard]] std::unique_ptr<name::name> const& target() const noexcept;
    
    /**
     * @brief returns the corresponding target columns.
     * @return the corresponding target columns
     * @return empty if they are not defined
     */
    [[nodiscard]] std::vector<std::unique_ptr<name::simple>>& target_columns() noexcept;

    /// @copydoc target_columns()
    [[nodiscard]] std::vector<std::unique_ptr<name::simple>> const& target_columns() const noexcept;
    
    /**
     * @brief returns referential action for update operations.
     * @return referential action for update operations
     * @return empty if it is not defined
     */
    [[nodiscard]] std::optional<action_type>& on_update() noexcept;

    /// @copydoc on_update()
    [[nodiscard]] std::optional<action_type> const& on_update() const noexcept;
    
    /**
     * @brief returns referential action for delete operations.
     * @return referential action for delete operations
     * @return empty if it is not defined
     */
    [[nodiscard]] std::optional<action_type>& on_delete() noexcept;

    /// @copydoc on_delete()
    [[nodiscard]] std::optional<action_type> const& on_delete() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(referential_constraint const& a, referential_constraint const& b) noexcept;
    
    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(referential_constraint const& a, referential_constraint const& b) noexcept;

protected:
    [[nodiscard]] bool equals(constraint const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    std::vector<std::unique_ptr<name::simple>> columns_ {};
    std::unique_ptr<name::name> target_ {};
    std::vector<std::unique_ptr<name::simple>> target_columns_ {};
    std::optional<action_type> on_update_ {};
    std::optional<action_type> on_delete_ {};
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, referential_constraint const& value);

} // namespace mizugaki::ast::statement
