#pragma once

#include <takatori/util/object_creator.h>

#include <mizugaki/ast/name/name.h>

#include "type.h"

namespace mizugaki::ast::type {

/**
 * @brief represents a user-defined type.
 */
class user_defined final : public type {

    using super = type;

public:
    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::user_defined;

    /**
     * @brief creates a new instance.
     * @param name the type name
     * @param region the node region
     */
    explicit user_defined(
            ::takatori::util::unique_object_ptr<name::name> name,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param name the type name
     * @param region the node region
     * @attention this will take copy of argument
     */
    explicit user_defined(
            name::name&& name,
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit user_defined(user_defined const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit user_defined(user_defined&& other, ::takatori::util::object_creator creator);

    [[nodiscard]] user_defined* clone(::takatori::util::object_creator creator) const& override;
    [[nodiscard]] user_defined* clone(::takatori::util::object_creator creator) && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the type name.
     * @return the type name
     */
    [[nodiscard]] ::takatori::util::unique_object_ptr<name::name>& name() noexcept;

    /// @brief name()
    [[nodiscard]] ::takatori::util::unique_object_ptr<name::name> const& name() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(user_defined const& a, user_defined const& b) noexcept;
    
    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(user_defined const& a, user_defined const& b) noexcept;
    
protected:
    [[nodiscard]] bool equals(type const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    ::takatori::util::unique_object_ptr<name::name> name_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, user_defined const& value);

} // namespace mizugaki::ast::type
