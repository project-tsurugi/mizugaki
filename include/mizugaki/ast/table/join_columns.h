#pragma once

#include <takatori/util/object_creator.h>

#include <mizugaki/ast/element.h>
#include <mizugaki/ast/common/vector.h>
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
    join_columns( // NOLINT: conversion constructor
            common::vector<::takatori::util::unique_object_ptr<name::simple>> columns,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit join_columns(join_columns const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit join_columns(join_columns&& other, ::takatori::util::object_creator creator);

    [[nodiscard]] join_columns* clone(::takatori::util::object_creator creator) const& override;
    [[nodiscard]] join_columns* clone(::takatori::util::object_creator creator) && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the join column names.
     * @return the join column names
     */
    [[nodiscard]] common::vector<::takatori::util::unique_object_ptr<name::simple>>& columns() noexcept;

    /// @copydoc columns()
    [[nodiscard]] common::vector<::takatori::util::unique_object_ptr<name::simple>> const& columns() const noexcept;

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

private:
    common::vector<::takatori::util::unique_object_ptr<name::simple>> columns_;
};

} // namespace mizugaki::ast::table
