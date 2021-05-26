#pragma once

#include <optional>

#include <cstddef>

#include <takatori/util/clone_tag.h>

#include <mizugaki/ast/common/regioned.h>

#include "type.h"

namespace mizugaki::ast::type {

/**
 * @brief represents a binary numeric type.
 */
class binary_numeric final : public type {

    using super = type;

public:
    /// @brief the type kind type.
    using type_kind_type = common::regioned<node_kind_type>;

    /// @brief the precision type.
    using precision_type = common::regioned<std::size_t>;

    /// @brief the flexible precision.
    static constexpr std::size_t flexible_precision = static_cast<std::size_t>(-1);

    /// @brief the available node kind of this.
    static constexpr kind_set tags {
            node_kind_type::binary_integer,
            node_kind_type::binary_float,
    };

    /**
     * @brief creates a new instance.
     * @param type_kind the binary_numeric type kind
     * @param precision the guaranteed number of binary significands
     * @param region the node region
     * @throws std::invalid_argument if kind is invalid
     * @see tags
     */
    explicit binary_numeric(
            type_kind_type type_kind,
            std::optional<precision_type> precision,
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit binary_numeric(::takatori::util::clone_tag_t, binary_numeric const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit binary_numeric(::takatori::util::clone_tag_t, binary_numeric&& other);

    [[nodiscard]] binary_numeric* clone() const& override;
    [[nodiscard]] binary_numeric* clone() && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the type kind.
     * @return the type kind
     */
    [[nodiscard]] type_kind_type& type_kind() noexcept;

    /// @copydoc type_kind()
    [[nodiscard]] type_kind_type const& type_kind() const noexcept;

    /**
     * @brief returns whether or not this type is flexible precision.
     * @return true if this is flexible precision
     * @return false otherwise
     * @see flexible_precision
     */
    [[nodiscard]] bool is_flexible_precision() const noexcept;

    /**
     * @brief returns the guaranteed number of binary significands.
     * @return the available number of bits in the value
     * @return empty if the precision is not defined
     * @return flexible_precision if this number has flexible precision
     */
    [[nodiscard]] std::optional<precision_type>& precision() noexcept;

    /// @copydoc precision()
    [[nodiscard]] std::optional<precision_type> const& precision() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(binary_numeric const& a, binary_numeric const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(binary_numeric const& a, binary_numeric const& b) noexcept;

protected:
    [[nodiscard]] bool equals(type const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    type_kind_type type_kind_;
    std::optional<precision_type> precision_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, binary_numeric const& value);

} // namespace mizugaki::ast::type
