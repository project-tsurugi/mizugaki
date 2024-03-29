#pragma once

#include <takatori/util/clone_tag.h>

#include <mizugaki/ast/name/name.h>

#include "expression.h"

namespace mizugaki::ast::scalar {

/**
 * @brief refers variable named elements.
 * @note `6.3 <value specification> and <target specification>`
 * @note `6.5 <identifier chain>`
 * @note `6.6 <column reference>`
 * @note `6.7 <SQL parameter reference>`
 * @note host_parameter_reference can handle host parameters instead of this class.
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
            std::unique_ptr<name::name> name,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param name the variable name
     * @param region the node region
     * @attention this will take copy of argument
     */
    explicit variable_reference(
            name::name&& name,
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit variable_reference(::takatori::util::clone_tag_t, variable_reference const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit variable_reference(::takatori::util::clone_tag_t, variable_reference&& other);

    [[nodiscard]] variable_reference* clone() const& override;
    [[nodiscard]] variable_reference* clone() && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the variable name.
     * @return the variable name
     */
    [[nodiscard]] std::unique_ptr<name::name>& name() noexcept;

    /// @brief name()
    [[nodiscard]] std::unique_ptr<name::name> const& name() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(variable_reference const& a, variable_reference const& b) noexcept;
    
    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(variable_reference const& a, variable_reference const& b) noexcept;
    
protected:
    [[nodiscard]] bool equals(expression const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    std::unique_ptr<name::name> name_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, variable_reference const& value);

} // namespace mizugaki::ast::scalar
