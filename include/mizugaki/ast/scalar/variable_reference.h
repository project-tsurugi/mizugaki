#pragma once

#include <takatori/util/object_creator.h>

#include <mizugaki/ast/name/name.h>

#include "expression.h"

namespace mizugaki::ast::scalar {

/**
 * @brief refers variable named elements.
 * @note `6.3 <value specification> and <target specification>`
 * @note `6.5 <identifier chain>`
 * @note `6.6 <column reference>`
 * @note `6.7 <SQL parameter reference>`
 */
class variable_reference final : public expression {

    using super = expression;

public:
    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::variable_reference;

    /**
     * @brief creates a new instance.
     * @param name the variable name
     * @param region the node region
     */
    explicit variable_reference(
            ::takatori::util::unique_object_ptr<name::name> name,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit variable_reference(variable_reference const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit variable_reference(variable_reference&& other, ::takatori::util::object_creator creator);

    [[nodiscard]] variable_reference* clone(::takatori::util::object_creator creator) const& override;
    [[nodiscard]] variable_reference* clone(::takatori::util::object_creator creator) && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the variable name.
     * @return the variable name
     */
    [[nodiscard]] ::takatori::util::unique_object_ptr<name::name>& name() noexcept;

    /// @brief name()
    [[nodiscard]] ::takatori::util::unique_object_ptr<name::name> const& name() const noexcept;

private:
    ::takatori::util::unique_object_ptr<name::name> name_;
};

} // namespace mizugaki::ast::scalar
