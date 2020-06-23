#pragma once

#include <takatori/util/object_creator.h>

#include <mizugaki/ast/element.h>
#include <mizugaki/ast/common/clone_wrapper.h>
#include <mizugaki/ast/common/vector.h>
#include <mizugaki/ast/common/rvalue_list.h>
#include <mizugaki/ast/name/simple.h>

namespace mizugaki::ast::query {

/**
 * @brief specifies corresponding columns (`CORRESPONDING`).
 * @note `7.12 <query expression>` - `<non-join query expression>` and `<non-join query term>`
 * @see binary_expression
 */
class corresponding_clause : public element {
public:
    /**
     * @brief creates a new instance.
     * @param column_names the corresponding column names
     * @param region the element region
     */
    explicit corresponding_clause(
            common::vector<::takatori::util::unique_object_ptr<name::simple>> column_names,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param column_names the corresponding column names
     * @param region the element region
     * @attention this will take a copy of argument
     */
    corresponding_clause(
            common::rvalue_list<name::simple> column_names,
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit corresponding_clause(corresponding_clause const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit corresponding_clause(corresponding_clause&& other, ::takatori::util::object_creator creator);

    /**
     * @brief returns the corresponding column names.
     * @return the column names
     * @return empty if they are not defined explicitly
     */
    [[nodiscard]] common::vector<::takatori::util::unique_object_ptr<name::simple>>& column_names() noexcept;

    /// @copydoc column_names()
    [[nodiscard]] common::vector<::takatori::util::unique_object_ptr<name::simple>> const& column_names() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(corresponding_clause const& a, corresponding_clause const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(corresponding_clause const& a, corresponding_clause const& b) noexcept;

    /**
     * @brief dumps structure information of the given value into the target acceptor.
     * @param acceptor the target acceptor
     * @param value the target value
     * @return the output
     */
    friend ::takatori::serializer::object_acceptor& operator<<(
            ::takatori::serializer::object_acceptor& acceptor,
            corresponding_clause const& value);

private:
    common::clone_wrapper<common::vector<::takatori::util::unique_object_ptr<name::simple>>> column_names_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, corresponding_clause const& value);

} // namespace mizugaki::ast::query
