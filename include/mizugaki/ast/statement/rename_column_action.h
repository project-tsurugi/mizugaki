#pragma once

#include <takatori/util/clone_tag.h>

#include <mizugaki/ast/node.h>
#include <mizugaki/ast/name/simple.h>
#include <mizugaki/ast/common/regioned.h>

#include "alter_table_action.h"

namespace mizugaki::ast::statement {

/**
 * @brief rename table column action of alter table statement.
 * @note This syntax element is out of SQL standard.
 */
class rename_column_action final : public alter_table_action {

    using super = alter_table_action;

public:
    /// @brief the kind of this element.
    static constexpr alter_table_action_kind tag = alter_table_action_kind::rename_column;

    /// @brief truth type with element region information.
    using bool_type = common::regioned<bool>;

    /**
     * @brief creates a new instance.
     * @param if_exists the flag indicates whether this statement is available only if the target table exists
     * @param column_name the target column name to rename
     * @param replacement the replacement name
     * @param region the element region
     */
    explicit rename_column_action(
            bool_type if_exists,
            std::unique_ptr<name::simple> column_name,
            std::unique_ptr<name::simple> replacement,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param column_name the target column name to rename
     * @param replacement the replacement name
     * @param if_exists the flag indicates whether this statement is available only if the target table exists
     * @param region the element region
     * @attention this will take copy of arguments
     */
    explicit rename_column_action(
            name::simple&& column_name,
            name::simple&& replacement,
            bool_type if_exists = { false },
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit rename_column_action(::takatori::util::clone_tag_t, rename_column_action const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit rename_column_action(::takatori::util::clone_tag_t, rename_column_action&& other);

    [[nodiscard]] rename_column_action* clone() const& override;
    [[nodiscard]] rename_column_action* clone() && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @return returns whether this statement is available only if the target column exists.
     * @return true if this statement is available only if the target column exists
     * @return false otherwise
     */
    [[nodiscard]] bool_type& if_exists() noexcept;

    /// @copydoc if_exists()
    [[nodiscard]] bool_type if_exists() const noexcept;

    /**
     * @brief returns the target column name to rename.
     * @return the target column name
     */
    [[nodiscard]] std::unique_ptr<name::simple>& column_name() noexcept;

    /// @copydoc column_name()
    [[nodiscard]] std::unique_ptr<name::simple> const& column_name() const noexcept;

    /**
     * @brief returns the replacement name.
     * @return the replacement name
     */
    [[nodiscard]] std::unique_ptr<name::simple>& replacement() noexcept;
    
    /// @copydoc replacement()
    [[nodiscard]] std::unique_ptr<name::simple> const& replacement() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(rename_column_action const& a, rename_column_action const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(rename_column_action const& a, rename_column_action const& b) noexcept;

protected:
    [[nodiscard]] bool equals(alter_table_action const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    bool_type if_exists_;
    std::unique_ptr<name::simple> column_name_ {};
    std::unique_ptr<name::simple> replacement_ {};
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, rename_column_action const& value);

} // namespace mizugaki::ast::statement
