#pragma once

#include <takatori/util/clone_tag.h>

#include <mizugaki/ast/element.h>
#include <mizugaki/ast/common/vector.h>
#include <mizugaki/ast/common/rvalue_list.h>
#include <mizugaki/ast/name/simple.h>

#include "join_specification.h"

namespace mizugaki::ast::table {

/**
 * @brief represents a join columns (`USING ...`).
 * @note `7.7 <joined table>` - `<named columns join>`
 */
class join_columns : public join_specification {

    using super = join_specification;

public:
    /// @brief the kind of this element.
    static constexpr join_specification_kind tag = join_specification_kind::columns;

    /**
     * @brief creates a new instance.
     * @param columns the column names
     * @param region the element region
     */
    explicit join_columns(
            std::vector<std::unique_ptr<name::simple>> columns,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param columns the column names
     * @param region the element region
     * @attention this will take a copy of argument
     */
    join_columns(
            common::rvalue_list<name::simple> columns,
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit join_columns(::takatori::util::clone_tag_t, join_columns const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit join_columns(::takatori::util::clone_tag_t, join_columns&& other);

    [[nodiscard]] join_columns* clone() const& override;
    [[nodiscard]] join_columns* clone() && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the join column names.
     * @return the join column names
     */
    [[nodiscard]] std::vector<std::unique_ptr<name::simple>>& columns() noexcept;

    /// @copydoc columns()
    [[nodiscard]] std::vector<std::unique_ptr<name::simple>> const& columns() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(join_columns const& a, join_columns const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(join_columns const& a, join_columns const& b) noexcept;

protected:
    [[nodiscard]] bool equals(join_specification const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    std::vector<std::unique_ptr<name::simple>> columns_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, join_columns const& value);

} // namespace mizugaki::ast::table
