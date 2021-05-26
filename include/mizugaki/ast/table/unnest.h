#pragma once

#include <takatori/util/clone_tag.h>

#include <mizugaki/ast/common/regioned.h>
#include <mizugaki/ast/scalar/expression.h>

#include "expression.h"
#include "correlation_clause.h"

namespace mizugaki::ast::table {

/**
 * @brief extract rows from the collection.
 * @note `7.6 <table reference>` - `<collection derived table>`
 */
class unnest final : public expression {

    using super = expression;

public:
    /// @brief the correlation declaration type.
    using correlation_type = correlation_clause;

    /// @brief truth type with element region information.
    using bool_type = common::regioned<bool>;

    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::unnest;

    /**
     * @brief creates a new instance.
     * @param expression the collection expression
     * @param with_ordinality whether or not `WITH ORDINALITY` is specified
     * @param correlation the correlation declaration
     * @param region the node region
     */
    explicit unnest(
            std::unique_ptr<scalar::expression> expression,
            bool_type with_ordinality,
            correlation_type correlation,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param expression the collection expression
     * @param correlation the correlation declaration
     * @param with_ordinality whether or not `WITH ORDINALITY` is specified
     * @param region the node region
     */
    explicit unnest(
            scalar::expression&& expression,
            correlation_type correlation,
            bool_type with_ordinality = false,
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit unnest(::takatori::util::clone_tag_t, unnest const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit unnest(::takatori::util::clone_tag_t, unnest&& other);

    [[nodiscard]] unnest* clone() const& override;
    [[nodiscard]] unnest* clone() && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the collection expression.
     * @return the collection expression
     */
    [[nodiscard]] std::unique_ptr<scalar::expression>& expression() noexcept;

    /// @copydoc expression()
    [[nodiscard]] std::unique_ptr<scalar::expression> const& expression() const noexcept;

    /**
     * @brief returns whether or not `WITH ORDINALITY` is specified.
     * @return true if `WITH ORDINALITY` is specified
     * @return false
     */
    [[nodiscard]] bool_type& with_ordinality() noexcept;

    /// @copydoc with_ordinality()
    [[nodiscard]] bool_type const& with_ordinality() const noexcept;

    /**
     * @brief returns the correlation declaration.
     * @return the correlation declaration
     * @return empty if there is no such the declaration
     */
    [[nodiscard]] correlation_type& correlation() noexcept;

    /// @copydoc correlation()
    [[nodiscard]] correlation_type const& correlation() const noexcept;

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
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    std::unique_ptr<scalar::expression> expression_;
    bool_type with_ordinality_;
    correlation_type correlation_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, unnest const& value);

} // namespace mizugaki::ast::table
