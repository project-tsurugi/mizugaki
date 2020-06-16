#pragma once

#include <takatori/util/object_creator.h>

#include <mizugaki/ast/common/regioned.h>
#include <mizugaki/ast/scalar/expression.h>

#include "expression.h"

namespace mizugaki::ast::table {

/**
 * @brief extract rows from the collection.
 * @note `7.6 <table reference>` - `<collection derived table>`
 */
class unnest final : public expression {

    using super = expression;

public:
    /// @brief truth type with element region information.
    using bool_type = common::regioned<bool>;

    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::unnest;

    /**
     * @brief creates a new instance.
     * @param expression the collection expression
     * @param with_ordinality whether or not `WITH ORDINALITY` is specified
     * @param region the node region
     */
    explicit unnest(
            ::takatori::util::unique_object_ptr<scalar::expression> expression,
            bool_type with_ordinality,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit unnest(unnest const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit unnest(unnest&& other, ::takatori::util::object_creator creator);

    [[nodiscard]] unnest* clone(::takatori::util::object_creator creator) const& override;
    [[nodiscard]] unnest* clone(::takatori::util::object_creator creator) && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the collection expression.
     * @return the collection expression
     */
    [[nodiscard]] ::takatori::util::unique_object_ptr<scalar::expression>& expression() noexcept;

    /// @copydoc expression()
    [[nodiscard]] ::takatori::util::unique_object_ptr<scalar::expression> const& expression() const noexcept;

    /**
     * @brief returns whether or not `WITH ORDINALITY` is specified.
     * @return true if `WITH ORDINALITY` is specified
     * @return false
     */
    [[nodiscard]] bool_type& with_ordinality() noexcept;

    /// @copydoc with_ordinality()
    [[nodiscard]] bool_type const& with_ordinality() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(unnest const& a, unnest const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(unnest const& a, unnest const& b) noexcept;

protected:
    [[nodiscard]] bool equals(table::expression const& other) const noexcept override;

private:
    ::takatori::util::unique_object_ptr<scalar::expression> expression_;
    bool_type with_ordinality_;
};

} // namespace mizugaki::ast::table
