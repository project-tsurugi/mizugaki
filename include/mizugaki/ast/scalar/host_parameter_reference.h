#pragma once

#include <takatori/util/object_creator.h>

#include <mizugaki/ast/name/simple.h>

#include "expression.h"

namespace mizugaki::ast::scalar {

/**
 * @brief refers host variables.
 * @note `6.3 <value specification> and <target specification>`
 */
class host_parameter_reference final : public expression {

    using super = expression;

public:
    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::host_parameter_reference;

    /**
     * @brief creates a new instance.
     * @param name the host parameter name (includes leading `:`)
     * @param region the node region
     */
    explicit host_parameter_reference(
            ::takatori::util::unique_object_ptr<name::simple> name,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param name the variable name
     * @param region the node region
     * @attention this will take copy of argument
     */
    explicit host_parameter_reference(
            name::simple&& name,
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit host_parameter_reference(host_parameter_reference const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit host_parameter_reference(host_parameter_reference&& other, ::takatori::util::object_creator creator);

    [[nodiscard]] host_parameter_reference* clone(::takatori::util::object_creator creator) const& override;
    [[nodiscard]] host_parameter_reference* clone(::takatori::util::object_creator creator) && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the variable name.
     * @return the variable name
     */
    [[nodiscard]] ::takatori::util::unique_object_ptr<name::simple>& name() noexcept;

    /// @brief name()
    [[nodiscard]] ::takatori::util::unique_object_ptr<name::simple> const& name() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(host_parameter_reference const& a, host_parameter_reference const& b) noexcept;
    
    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(host_parameter_reference const& a, host_parameter_reference const& b) noexcept;
    
protected:
    [[nodiscard]] bool equals(expression const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    ::takatori::util::unique_object_ptr<name::simple> name_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, host_parameter_reference const& value);

} // namespace mizugaki::ast::scalar
