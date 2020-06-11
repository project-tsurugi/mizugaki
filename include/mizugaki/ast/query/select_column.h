#pragma once

#include <takatori/util/object_creator.h>

#include <mizugaki/ast/element.h>
#include <mizugaki/ast/name/simple.h>
#include <mizugaki/ast/scalar/expression.h>

#include "select_element.h"

namespace mizugaki::ast::query {

/**
 * @brief explicit column declaration in query expression.
 * @note `7.11 <query specification>` - `<derived column>`
 */
class select_column : public select_element {

    using super = select_element;

public:
    /// @brief the kind of this element.
    static constexpr select_element_kind tag = select_element_kind::column;

    /**
     * @brief creates a new instance.
     * @param value the column value
     * @param name the optional column name
     * @param region the element region
     */
    select_column( // NOLINT: conversion constructor
            ::takatori::util::unique_object_ptr<scalar::expression> value,
            ::takatori::util::unique_object_ptr<name::simple> name = {},
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit select_column(select_column const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit select_column(select_column&& other, ::takatori::util::object_creator creator);

    [[nodiscard]] select_column* clone(::takatori::util::object_creator creator) const& override;
    [[nodiscard]] select_column* clone(::takatori::util::object_creator creator) && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the column value expression.
     * @return the column value expression
     */
    [[nodiscard]] ::takatori::util::unique_object_ptr<scalar::expression>& value() noexcept;

    /// @copydoc value()
    [[nodiscard]] ::takatori::util::unique_object_ptr<scalar::expression> const& value() const noexcept;

    /**
     * @brief returns the column name.
     * @return the column name
     * @return empty there is no explicit column name
     */
    [[nodiscard]] ::takatori::util::unique_object_ptr<name::simple>& name() noexcept;

    /// @copydoc name()
    [[nodiscard]] ::takatori::util::unique_object_ptr<name::simple> const& name() const noexcept;

private:
    ::takatori::util::unique_object_ptr<scalar::expression> value_;
    ::takatori::util::unique_object_ptr<name::simple> name_;
};

} // namespace mizugaki::ast::query