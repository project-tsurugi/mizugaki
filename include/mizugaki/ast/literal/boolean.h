#pragma once

#include <takatori/util/clone_tag.h>

#include <mizugaki/ast/common/regioned.h>

#include "literal.h"
#include "boolean_kind.h"

namespace mizugaki::ast::literal {

/**
 * @brief represents a boolean value.
 */
class boolean final : public literal {

    using super = literal;

public:
    /// @brief the boolean kind type.
    using value_type = boolean_kind;

    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::boolean;

    /**
     * @brief creates a new instance.
     * @param value the boolean kind
     * @param region the node region
     */
    explicit boolean(
            value_type value,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param value the boolean value
     * @param region the node region
     */
    explicit boolean(
            bool value,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param value always nullptr that represents unknown
     * @param region the node region
     */
    explicit boolean(
            std::nullptr_t value,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit boolean(::takatori::util::clone_tag_t, boolean const& other) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit boolean(::takatori::util::clone_tag_t, boolean&& other) noexcept;

    [[nodiscard]] boolean* clone() const& override;
    [[nodiscard]] boolean* clone() && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the boolean value kind.
     * @return the boolean value kind
     */
    [[nodiscard]] value_type& value() noexcept;

    /// @copydoc value()
    [[nodiscard]] value_type value() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(boolean const& a, boolean const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(boolean const& a, boolean const& b) noexcept;

protected:
    [[nodiscard]] bool equals(literal const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    value_type value_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, boolean const& value);

} // namespace mizugaki::ast::literal
