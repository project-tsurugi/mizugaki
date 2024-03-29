#pragma once

#include <takatori/util/clone_tag.h>
#include <takatori/util/rvalue_ptr.h>

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
    explicit select_column(
            std::unique_ptr<scalar::expression> value,
            std::unique_ptr<name::simple> name = {},
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param value the column value
     * @param name the optional column name
     * @param region the element region
     * @attention this will take copy of arguments
     */
    explicit select_column(
            scalar::expression&& value,
            ::takatori::util::rvalue_ptr<name::simple> name = {},
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit select_column(::takatori::util::clone_tag_t, select_column const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit select_column(::takatori::util::clone_tag_t, select_column&& other);

    [[nodiscard]] select_column* clone() const& override;
    [[nodiscard]] select_column* clone() && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the column value expression.
     * @return the column value expression
     */
    [[nodiscard]] std::unique_ptr<scalar::expression>& value() noexcept;

    /// @copydoc value()
    [[nodiscard]] std::unique_ptr<scalar::expression> const& value() const noexcept;

    /**
     * @brief returns the column name.
     * @return the column name
     * @return empty there is no explicit column name
     */
    [[nodiscard]] std::unique_ptr<name::simple>& name() noexcept;

    /// @copydoc name()
    [[nodiscard]] std::unique_ptr<name::simple> const& name() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(select_column const& a, select_column const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(select_column const& a, select_column const& b) noexcept;

protected:
    [[nodiscard]] bool equals(select_element const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    std::unique_ptr<scalar::expression> value_;
    std::unique_ptr<name::simple> name_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, select_column const& value);

} // namespace mizugaki::ast::query
