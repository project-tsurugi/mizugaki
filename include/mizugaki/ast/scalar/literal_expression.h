#pragma once

#include <takatori/util/clone_tag.h>

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
            std::unique_ptr<literal::literal> value,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param value the literal value
     * @param region the node region
     * @attention this will take a copy of argument
     */
    explicit literal_expression(
            literal::literal&& value,
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit literal_expression(::takatori::util::clone_tag_t, literal_expression const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit literal_expression(::takatori::util::clone_tag_t, literal_expression&& other);

    [[nodiscard]] literal_expression* clone() const& override;
    [[nodiscard]] literal_expression* clone() && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the value.
     * @return the value
     */
    [[nodiscard]] std::unique_ptr<literal::literal>& value() noexcept;

    /// @brief value()
    [[nodiscard]] std::unique_ptr<literal::literal> const& value() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(literal_expression const& a, literal_expression const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(literal_expression const& a, literal_expression const& b) noexcept;

protected:
    [[nodiscard]] bool equals(expression const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    std::unique_ptr<literal::literal> value_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, literal_expression const& value);

} // namespace mizugaki::ast::scalar
