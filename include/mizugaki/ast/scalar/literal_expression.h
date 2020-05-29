#pragma once

#include <takatori/util/object_creator.h>

#include <mizugaki/ast/common/regioned.h>
#include <mizugaki/ast/literal/literal.h>

#include "expression.h"

namespace mizugaki::ast::scalar {

/**
 * @brief represents a literal values.
 * @note `5.3 <literal>`
 * @note `6.3 <value specification> and <target specification>`
 */
class literal_expression final : public expression {

    using super = expression;

public:
    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::literal_expression;

    /**
     * @brief creates a new instance.
     * @param value the literal value
     * @param region the node region
     */
    explicit literal_expression(
            ::takatori::util::unique_object_ptr<literal::literal> value,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit literal_expression(literal_expression const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit literal_expression(literal_expression&& other, ::takatori::util::object_creator creator);

    [[nodiscard]] literal_expression* clone(::takatori::util::object_creator creator) const& override;
    [[nodiscard]] literal_expression* clone(::takatori::util::object_creator creator) && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the value.
     * @return the value
     */
    [[nodiscard]] ::takatori::util::unique_object_ptr<literal::literal>& value() noexcept;

    /// @brief value()
    [[nodiscard]] ::takatori::util::unique_object_ptr<literal::literal> const& value() const noexcept;

private:
    ::takatori::util::unique_object_ptr<literal::literal> value_;
};

} // namespace mizugaki::ast::scalar
