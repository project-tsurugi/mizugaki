#pragma once

#include <takatori/util/clone_tag.h>

#include <mizugaki/ast/node.h>
#include <mizugaki/ast/name/simple.h>

#include "alter_table_action.h"

namespace mizugaki::ast::statement {

/**
 * @brief rename table action of alter table statement.
 * @note This syntax element is out of SQL standard.
 */
class rename_table_action final : public alter_table_action {

    using super = alter_table_action;

public:
    /// @brief the kind of this element.
    static constexpr alter_table_action_kind tag = alter_table_action_kind::rename_table;

    /**
     * @brief creates a new instance.
     * @param replacement the replacement name
     * @param region the element region
     */
    explicit rename_table_action(
            std::unique_ptr<name::simple> replacement,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param replacement the replacement name
     * @param region the element region
     * @attention this will take copy of arguments
     */
    explicit rename_table_action(
            name::simple&& replacement,
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit rename_table_action(::takatori::util::clone_tag_t, rename_table_action const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit rename_table_action(::takatori::util::clone_tag_t, rename_table_action&& other);

    [[nodiscard]] rename_table_action* clone() const& override;
    [[nodiscard]] rename_table_action* clone() && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

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
    friend bool operator==(rename_table_action const& a, rename_table_action const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(rename_table_action const& a, rename_table_action const& b) noexcept;

protected:
    [[nodiscard]] bool equals(alter_table_action const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    std::unique_ptr<name::simple> replacement_ {};
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, rename_table_action const& value);

} // namespace mizugaki::ast::statement
