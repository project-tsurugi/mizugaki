#pragma once

#include <initializer_list>
#include <vector>

#include <takatori/util/clone_tag.h>

#include <mizugaki/ast/node.h>
#include <mizugaki/ast/name/simple.h>
#include <mizugaki/ast/type/type.h>

#include "table_element.h"
#include "column_constraint_definition.h"

namespace mizugaki::ast::statement {

/**
 * @brief table column definitions.
 * @note `11.4 <column definition>`
 */
class column_definition final : public table_element {

    using super = table_element;

public:
    /// @brief the kind of this element.
    static constexpr table_element_kind tag = table_element_kind::column_definition;

    /**
     * @brief creates a new instance.
     * @param name the column name
     * @param type the column type
     * @param constraints the column constraints, including default value
     * @param description the region of description comment
     * @param region the element region
     */
    explicit column_definition(
            std::unique_ptr<name::simple> name,
            std::unique_ptr<type::type> type,
            std::vector<column_constraint_definition> constraints = {},
            region_type description = {},
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param name the column name
     * @param type the column type
     * @param constraints the column constraints, including default value
     * @param description the region of description comment
     * @param region the element region
     * @attention this will take copy of arguments
     */
    explicit column_definition(
            name::simple&& name,
            type::type&& type,
            std::initializer_list<column_constraint_definition> constraints = {},
            region_type description = {},
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit column_definition(::takatori::util::clone_tag_t, column_definition const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit column_definition(::takatori::util::clone_tag_t, column_definition&& other);

    [[nodiscard]] column_definition* clone() const& override;
    [[nodiscard]] column_definition* clone() && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the column name.
     * @return the column name
     */
    [[nodiscard]] std::unique_ptr<name::simple>& name() noexcept;
    
    /// @copydoc name()
    [[nodiscard]] std::unique_ptr<name::simple> const& name() const noexcept;
    
    /**
     * @brief returns the column type.
     * @return the column type
     */
    [[nodiscard]] std::unique_ptr<type::type>& type() noexcept;
    
    /// @copydoc type()
    [[nodiscard]] std::unique_ptr<type::type> const& type() const noexcept;
    
    /**
     * @brief returns the column constraints.
     * @note This may include the column's default value.
     * @return list of column constraints
     */
    [[nodiscard]] std::vector<column_constraint_definition>& constraints() noexcept;
    
    /// @copydoc constraints()
    [[nodiscard]] std::vector<column_constraint_definition> const& constraints() const noexcept;

    /**
     * @brief returns the region of description comment for this definition.
     * @return the description comment region
     * @return the empty region if not specified
     */
    [[nodiscard]] region_type& description() noexcept;

    /// @copydoc description()
    [[nodiscard]] region_type const& description() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(column_definition const& a, column_definition const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(column_definition const& a, column_definition const& b) noexcept;

protected:
    [[nodiscard]] bool equals(table_element const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    std::unique_ptr<name::simple> name_ {};
    std::unique_ptr<type::type> type_ {};
    std::vector<column_constraint_definition> constraints_ {};
    region_type description_ {};
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, column_definition const& value);

} // namespace mizugaki::ast::statement
