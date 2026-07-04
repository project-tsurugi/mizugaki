#pragma once

#include <memory>

#include <takatori/util/clone_tag.h>

#include <mizugaki/ast/name/name.h>
#include <mizugaki/ast/common/regioned.h>

#include "statement.h"
#include "alter_table_action.h"

namespace mizugaki::ast::statement {

/**
 * @brief modify table definitions.
 * @note `11.10 <alter table statement>`
 */
class alter_table_statement final : public statement {

    using super = statement;

public:
    /// @brief the kind of this element.
    static constexpr node_kind_type tag = node_kind_type::alter_table_statement;

    /// @brief truth type with element region information.
    using bool_type = common::regioned<bool>;

    /**
     * @brief creates a new instance.
     * @param if_exists the flag indicates whether this statement is available only if the target table exists
     * @param name the target table name
     * @param action the alter table action
     * @param region the node region
     */
    explicit alter_table_statement(
            bool_type if_exists,
            std::unique_ptr<name::name> name,
            std::unique_ptr<alter_table_action> action,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param name the target table name
     * @param action the alter table action
     * @param if_exists the flag indicates whether this statement is available only if the target table exists
     * @param region the node region
     * @attention this will take copy of arguments
     */
    explicit alter_table_statement(
            name::name&& name,
            alter_table_action&& action,
            bool_type if_exists = { false },
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit alter_table_statement(::takatori::util::clone_tag_t, alter_table_statement const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit alter_table_statement(::takatori::util::clone_tag_t, alter_table_statement&& other);

    [[nodiscard]] alter_table_statement* clone() const& override;
    [[nodiscard]] alter_table_statement* clone() && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @return returns whether this statement is available only if the target table exists.
     * @return true if this statement is available only if the target table exists
     * @return false otherwise
     */
    [[nodiscard]] bool_type& if_exists() noexcept;

    /// @copydoc if_exists()
    [[nodiscard]] bool_type if_exists() const noexcept;

    /**
     * @brief returns the target table name.
     * @return the target table name
     */
    [[nodiscard]] std::unique_ptr<name::name>& name() noexcept;

    /// @brief name()
    [[nodiscard]] std::unique_ptr<name::name> const& name() const noexcept;

    /**
     * @brief returns the alter table action.
     * @return the alter table action
     */
    [[nodiscard]] std::unique_ptr<alter_table_action>& action() noexcept;

    /// @copydoc action()
    [[nodiscard]] std::unique_ptr<alter_table_action> const& action() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(alter_table_statement const& a, alter_table_statement const& b) noexcept;
    
    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(alter_table_statement const& a, alter_table_statement const& b) noexcept;

protected:
    [[nodiscard]] bool equals(statement const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    bool_type if_exists_;
    std::unique_ptr<name::name> name_;
    std::unique_ptr<alter_table_action> action_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, alter_table_statement const& value);

} // namespace mizugaki::ast::statement
