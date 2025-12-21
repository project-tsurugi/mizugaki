#pragma once

#include <takatori/util/clone_tag.h>

#include <mizugaki/ast/element.h>
#include <mizugaki/ast/scalar/expression.h>

#include "join_specification.h"

namespace mizugaki::ast::table {

/**
 * @brief represents a join condition (`ON ...`).
 * @note `7.7 <joined table>` - `<join condition>`
 */
class join_condition : public join_specification {

    using super = join_specification;

public:
    /// @brief the kind of this element.
    static constexpr join_specification_kind tag = join_specification_kind::condition;

    /**
     * @brief creates a new instance.
     * @param expression the condition expression
     * @param region the element region
     */
    explicit join_condition(
            std::unique_ptr<scalar::expression> expression,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param expression the condition expression
     * @param region the element region
     * @attention this will take a copy of argument
     */
    explicit join_condition(
            scalar::expression&& expression,
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit join_condition(::takatori::util::clone_tag_t, join_condition const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit join_condition(::takatori::util::clone_tag_t, join_condition&& other);

    [[nodiscard]] join_condition* clone() const& override;
    [[nodiscard]] join_condition* clone() && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the condition expression.
     * @return the condition expression
     */
    [[nodiscard]] std::unique_ptr<scalar::expression>& expression() noexcept;

    /// @copydoc expression()
    [[nodiscard]] std::unique_ptr<scalar::expression> const& expression() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(join_condition const& a, join_condition const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(join_condition const& a, join_condition const& b) noexcept;

protected:
    [[nodiscard]] bool equals(join_specification const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    std::unique_ptr<scalar::expression> expression_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, join_condition const& value);

} // namespace mizugaki::ast::table
