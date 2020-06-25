#pragma once

#include <memory>
#include <optional>

#include <takatori/util/clone_tag.h>

#include <mizugaki/ast/common/regioned.h>
#include <mizugaki/ast/scalar/expression.h>

#include "constraint.h"
#include "identity_generation_type.h"

namespace mizugaki::ast::statement {

/**
 * @brief identity column constraints.
 * @note `[SQL-2003] 11.4 <column definition>` - `<identity column specification>`
 */
class identity_constraint final : public constraint {

    using super = constraint;

public:
    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::identity_column;

    /// @brief the generation type.
    using generation_type = common::regioned<identity_generation_type>;

    /// @brief the bool type.
    using bool_type = common::regioned<bool>;

    /**
     * @brief creates a new object.
     * @param generation the generation type 
     * @param initial_value the initial value, or empty if it is not defined
     * @param increment_value the increment value, or empty if it is not defined
     * @param min_value the minimum value, or empty if it is not defined
     * @param max_value the maximum value, or empty if it is not defined
     * @param cycle whether or not to enable value overflow, or empty if it is not defined
     * @param region the node region
     */
    explicit identity_constraint(
            generation_type generation,
            std::unique_ptr<scalar::expression> initial_value = {},
            std::unique_ptr<scalar::expression> increment_value = {},
            std::unique_ptr<scalar::expression> min_value = {},
            std::unique_ptr<scalar::expression> max_value = {},
            std::optional<bool_type> cycle = {},
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit identity_constraint(::takatori::util::clone_tag_t, identity_constraint const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit identity_constraint(::takatori::util::clone_tag_t, identity_constraint&& other);

    [[nodiscard]] identity_constraint* clone() const& override;
    [[nodiscard]] identity_constraint* clone() && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the generation type.
     * @return the generation type
     */
    [[nodiscard]] generation_type& generation() noexcept;
    
    /// @copydoc generation()
    [[nodiscard]] generation_type const& generation() const noexcept;

    /**
     * @brief returns the initial value.
     * @return the initial value
     * @return empty if it is not defined
     */
    [[nodiscard]] std::unique_ptr<scalar::expression>& initial_value() noexcept;

    /// @copydoc initial_value()
    [[nodiscard]] std::unique_ptr<scalar::expression> const& initial_value() const noexcept;

    /**
     * @brief returns the increment value.
     * @return the increment value
     * @return empty if it is not defined
     */
    [[nodiscard]] std::unique_ptr<scalar::expression>& increment_value() noexcept;

    /// @copydoc increment_value()
    [[nodiscard]] std::unique_ptr<scalar::expression> const& increment_value() const noexcept;

    /**
     * @brief returns the minimum value.
     * @return the minimum value
     * @return empty if it is not defined
     */
    [[nodiscard]] std::unique_ptr<scalar::expression>& min_value() noexcept;

    /// @copydoc min_value()
    [[nodiscard]] std::unique_ptr<scalar::expression> const& min_value() const noexcept;

    /**
     * @brief returns the maximum value.
     * @return the maximum value
     * @return empty if it is not defined
     */
    [[nodiscard]] std::unique_ptr<scalar::expression>& max_value() noexcept;

    /// @copydoc max_value()
    [[nodiscard]] std::unique_ptr<scalar::expression> const& max_value() const noexcept;

    /**
     * @brief returns whether or not the value can overflow.
     * @return true if value overflow is enabled on this sequence
     * @return  false otherwise
     */
    [[nodiscard]] std::optional<bool_type>& cycle() noexcept;
    
    /// @copydoc cycle()
    [[nodiscard]] std::optional<bool_type> const& cycle() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(identity_constraint const& a, identity_constraint const& b) noexcept;
    
    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(identity_constraint const& a, identity_constraint const& b) noexcept;

protected:
    [[nodiscard]] bool equals(constraint const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    generation_type generation_ {};
    std::unique_ptr<scalar::expression> initial_value_ {};
    std::unique_ptr<scalar::expression> increment_value_ {};
    std::unique_ptr<scalar::expression> min_value_ {};
    std::unique_ptr<scalar::expression> max_value_ {};
    std::optional<bool_type> cycle_ {};
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, identity_constraint const& value);

} // namespace mizugaki::ast::statement
