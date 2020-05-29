#pragma once

#include <takatori/util/object_creator.h>

#include <mizugaki/ast/common/vector.h>
#include <mizugaki/ast/scalar/expression.h>

#include "expression.h"
#include "with_element.h"

namespace mizugaki::ast::query {

/**
 * @brief declares queries (`WITH`).
 * @note `7.12 <query expression>` - `<with clause>`
 */
class with_expression final : public expression {

    using super = expression;

public:
    /// @brief the element type.
    using element_type = with_element;
    
    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::with_expression;

    /**
     * @brief creates a new instance.
     * @param elements the named query declarations
     * @param body the body expression
     * @param region the node region
     */
    explicit with_expression(
            common::vector<element_type> elements,
            ::takatori::util::unique_object_ptr<expression> body,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit with_expression(with_expression const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit with_expression(with_expression&& other, ::takatori::util::object_creator creator);

    [[nodiscard]] with_expression* clone(::takatori::util::object_creator creator) const& override;
    [[nodiscard]] with_expression* clone(::takatori::util::object_creator creator) && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the named query declarations.
     * @return the named query declarations
     */
    [[nodiscard]] common::vector<element_type>& elements() noexcept;

    /// @brief elements()
    [[nodiscard]] common::vector<element_type> const& elements() const noexcept;

    /**
     * @brief returns the body expression.
     * @return the body expression
     */
    [[nodiscard]] ::takatori::util::unique_object_ptr<expression>& body() noexcept;

    /// @copydoc body()
    [[nodiscard]] ::takatori::util::unique_object_ptr<expression> const& body() const noexcept;

private:
    common::vector<element_type> elements_;
    ::takatori::util::unique_object_ptr<expression> body_;
};

} // namespace mizugaki::ast::query
