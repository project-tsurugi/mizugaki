#pragma once

#include <mizugaki/ast/common/regioned.h>

#include "literal.h"
#include "boolean_kind.h"

namespace mizugaki::ast::literal {

/**
 * @brief represents a boolean value.
 */
class boolean final : public literal {

    using super = literal;

public:
    /// @brief the boolean kind type.
    using value_type = boolean_kind;

    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::boolean;

    /**
     * @brief creates a new instance.
     * @param value the boolean kind
     * @param region the node region
     */
    explicit boolean(
            value_type value,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit boolean(boolean const& other, ::takatori::util::object_creator creator) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit boolean(boolean&& other, ::takatori::util::object_creator creator) noexcept;

    [[nodiscard]] boolean* clone(::takatori::util::object_creator creator) const& override;
    [[nodiscard]] boolean* clone(::takatori::util::object_creator creator) && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the boolean value kind.
     * @return the boolean value kind
     */
    [[nodiscard]] value_type& value() noexcept;

    /// @copydoc value()
    [[nodiscard]] value_type value() const noexcept;

private:
    value_type value_;
};

} // namespace mizugaki::ast::literal
