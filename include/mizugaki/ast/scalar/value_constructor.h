#pragma once

#include <takatori/util/object_creator.h>

#include <mizugaki/ast/common/regioned.h>
#include <mizugaki/ast/common/vector.h>

#include "expression.h"
#include "value_constructor_kind.h"

namespace mizugaki::ast::scalar {

/**
 * @brief constructs a row value.
 * @note `7.1 <row value constructor>`
 */
class value_constructor final : public expression {

    using super = expression;

public:
    /// @brief the constructor kind type.
    using constructor_kind_type = common::regioned<value_constructor_kind>;

    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::value_constructor;

    /**
     * @brief creates a new instance.
     * @param constructor_kind the construction target kind
     * @param elements the row value elements
     * @param region the node region
     */
    explicit value_constructor(
            constructor_kind_type constructor_kind,
            common::vector<operand_type> elements,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit value_constructor(value_constructor const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit value_constructor(value_constructor&& other, ::takatori::util::object_creator creator);

    [[nodiscard]] value_constructor* clone(::takatori::util::object_creator creator) const& override;
    [[nodiscard]] value_constructor* clone(::takatori::util::object_creator creator) && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the constructor kind.
     * @return the constructor kind
     */
    [[nodiscard]] constructor_kind_type& constructor_kind() noexcept;

    /// @copydoc constructor_kind()
    [[nodiscard]] constructor_kind_type const& constructor_kind() const noexcept;

    /**
     * @brief returns the row value elements.
     * @return the row value elements
     */
    [[nodiscard]] common::vector<operand_type>& elements() noexcept;

    /// @copydoc elements()
    [[nodiscard]] common::vector<operand_type> const& elements() const noexcept;

private:
    constructor_kind_type constructor_kind_;
    common::vector<operand_type> elements_;
};

} // namespace mizugaki::ast::scalar
