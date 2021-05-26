#pragma once

#include <takatori/util/clone_tag.h>

#include <mizugaki/ast/common/regioned.h>
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

    /// @brief truth type with element region information.
    using bool_type = common::regioned<bool>;

    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::with_expression;

    /**
     * @brief creates a new instance.
     * @param is_recursive whether or not `RECURSIVE` is specified
     * @param elements the named query declarations
     * @param expression the body expression
     * @param region the node region
     */
    explicit with_expression(
            bool_type is_recursive,
            std::vector<element_type> elements,
            std::unique_ptr<ast::query::expression> expression,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param elements the named query declarations
     * @param expression the body expression
     * @param region the node region
     * @attention this will take copy of arguments
     */
    explicit with_expression(
            std::initializer_list<element_type> elements,
            ast::query::expression&& expression,
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param is_recursive whether or not `RECURSIVE` is specified
     * @param elements the named query declarations
     * @param expression the body expression
     * @param region the node region
     * @attention this will take copy of arguments
     */
    explicit with_expression(
            bool_type is_recursive,
            std::initializer_list<element_type> elements,
            ast::query::expression&& expression,
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit with_expression(::takatori::util::clone_tag_t, with_expression const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit with_expression(::takatori::util::clone_tag_t, with_expression&& other);

    [[nodiscard]] with_expression* clone() const& override;
    [[nodiscard]] with_expression* clone() && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the named query declarations.
     * @return the named query declarations
     */
    [[nodiscard]] std::vector<element_type>& elements() noexcept;

    /// @brief elements()
    [[nodiscard]] std::vector<element_type> const& elements() const noexcept;

    /**
     * @brief returns the body expression.
     * @return the body expression
     */
    [[nodiscard]] std::unique_ptr<ast::query::expression>& expression() noexcept;

    /// @copydoc expression()
    [[nodiscard]] std::unique_ptr<ast::query::expression> const& expression() const noexcept;

    /**
     * @brief returns whether or not `RECURSIVE` is specified.
     * @return true if `RECURSIVE` is specified
     * @return false
     */
    [[nodiscard]] bool_type& is_recursive() noexcept;

    /// @copydoc is_recursive()
    [[nodiscard]] bool_type const& is_recursive() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(with_expression const& a, with_expression const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(with_expression const& a, with_expression const& b) noexcept;

protected:
    [[nodiscard]] bool equals(ast::query::expression const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    bool_type is_recursive_;
    std::vector<element_type> elements_;
    std::unique_ptr<ast::query::expression> expression_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, with_expression const& value);

} // namespace mizugaki::ast::query
