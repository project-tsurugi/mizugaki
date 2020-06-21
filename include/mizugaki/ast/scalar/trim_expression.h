#pragma once

#include <optional>

#include <takatori/util/object_creator.h>

#include <mizugaki/ast/common/regioned.h>

#include "expression.h"
#include "trim_specification.h"

namespace mizugaki::ast::scalar {

/**
 * @brief represents `TRIM` expression.
 * @note `6.18 <string value function>` - `<trim function>`
 * @see trim_specification
 */
class trim_expression final : public expression {

    using super = expression;

public:
    /// @brief the trim specification type.
    using specification_type = common::regioned<trim_specification>;

    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::trim_expression;

    /**
     * @brief creates a new instance.
     * @param specification the trim specification
     * @param character the optional trim character term
     * @param source the source term
     * @param region the node region
     */
    explicit trim_expression(
            std::optional<specification_type> specification,
            operand_type character,
            operand_type source,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit trim_expression(trim_expression const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit trim_expression(trim_expression&& other, ::takatori::util::object_creator creator);

    [[nodiscard]] trim_expression* clone(::takatori::util::object_creator creator) const& override;
    [[nodiscard]] trim_expression* clone(::takatori::util::object_creator creator) && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the trim specification.
     * @return the the trim specification
     * @return empty if it is not specified
     */
    [[nodiscard]] std::optional<specification_type>& specification() noexcept;

    /// @copydoc specification()
    [[nodiscard]] std::optional<specification_type> const& specification() const noexcept;

    /**
     * @brief returns the character term.
     * @return the character term
     * @return empty if it is not specified
     */
    [[nodiscard]] operand_type& character() noexcept;

    /// @copydoc character()
    [[nodiscard]] operand_type const& character() const noexcept;

    /**
     * @brief returns the source term.
     * @return the source term
     */
    [[nodiscard]] operand_type& source() noexcept;

    /// @copydoc source()
    [[nodiscard]] operand_type const& source() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(trim_expression const& a, trim_expression const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(trim_expression const& a, trim_expression const& b) noexcept;

protected:
    [[nodiscard]] bool equals(expression const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    std::optional<specification_type> specification_;
    operand_type character_;
    operand_type source_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, trim_expression const& value);

} // namespace mizugaki::ast::scalar
