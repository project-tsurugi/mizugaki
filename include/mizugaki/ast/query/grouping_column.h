#pragma once

#include <takatori/util/object_creator.h>

#include <mizugaki/ast/element.h>
#include <mizugaki/ast/common/clone_wrapper.h>
#include <mizugaki/ast/name/name.h>
#include <mizugaki/ast/scalar/expression.h>

#include "grouping_element.h"

namespace mizugaki::ast::query {

/**
 * @brief represents a grouping column expression.
 * @note `7.9 <group by clause>` - `<grouping column reference>`
 */
class grouping_column : public grouping_element {

    using super = grouping_element;

public:
    /// @brief the kind of this element.
    static constexpr grouping_element_kind tag = grouping_element_kind::column;

    /**
     * @brief creates a new instance.
     * @param column the column expression
     * @param collation the optional grouping collation
     * @param region the element region
     */
    grouping_column( // NOLINT: conversion constructor
            ::takatori::util::unique_object_ptr<scalar::expression> column,
            ::takatori::util::unique_object_ptr<name::name> collation = {},
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit grouping_column(grouping_column const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit grouping_column(grouping_column&& other, ::takatori::util::object_creator creator);

    [[nodiscard]] grouping_column* clone(::takatori::util::object_creator creator) const& override;
    [[nodiscard]] grouping_column* clone(::takatori::util::object_creator creator) && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the column expression.
     * @return the column expression
     */
    [[nodiscard]] ::takatori::util::unique_object_ptr<scalar::expression>& column() noexcept;

    /// @copydoc column()
    [[nodiscard]] ::takatori::util::unique_object_ptr<scalar::expression> const& column() const noexcept;

    /**
     * @brief returns the grouping collation.
     * @return the grouping collation
     * @return empty there is no explicit collation
     */
    [[nodiscard]] ::takatori::util::unique_object_ptr<name::name>& collation() noexcept;

    /// @copydoc collation()
    [[nodiscard]] ::takatori::util::unique_object_ptr<name::name> const& collation() const noexcept;

private:
    ::takatori::util::unique_object_ptr<scalar::expression> column_;
    ::takatori::util::unique_object_ptr<name::name> collation_;
};

} // namespace mizugaki::ast::query