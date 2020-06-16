#pragma once

#include <takatori/util/object_creator.h>

#include <mizugaki/ast/element.h>
#include <mizugaki/ast/scalar/expression.h>

#include "select_element.h"

namespace mizugaki::ast::query {

/**
 * @brief field enumerations in select list.
 * @note `7.11 <query specification>` - `<all fields reference>`
 */
class select_all_fields : public select_element {

    using super = select_element;

public:
    /// @brief the kind of this element.
    static constexpr select_element_kind tag = select_element_kind::all_fields;

    /**
     * @brief creates a new instance.
     * @param value the target value
     * @param region the element region
     */
    select_all_fields( // NOLINT: conversion constructor
            ::takatori::util::unique_object_ptr<scalar::expression> value,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit select_all_fields(select_all_fields const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit select_all_fields(select_all_fields&& other, ::takatori::util::object_creator creator);

    [[nodiscard]] select_all_fields* clone(::takatori::util::object_creator creator) const& override;
    [[nodiscard]] select_all_fields* clone(::takatori::util::object_creator creator) && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the target value.
     * @return the target value
     */
    [[nodiscard]] ::takatori::util::unique_object_ptr<scalar::expression>& value() noexcept;

    /// @copydoc value()
    [[nodiscard]] ::takatori::util::unique_object_ptr<scalar::expression> const& value() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(select_all_fields const& a, select_all_fields const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(select_all_fields const& a, select_all_fields const& b) noexcept;

protected:
    [[nodiscard]] bool equals(select_element const& other) const noexcept override;

private:
    ::takatori::util::unique_object_ptr<scalar::expression> value_;
};

} // namespace mizugaki::ast::query
