#pragma once

#include <optional>

#include <cstddef>

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
    using precision_type = std::size_t;

    /// @brief the flexible precision.
    static constexpr precision_type flexible_precision = static_cast<precision_type>(-1);

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
     * @param creator the object creator
     */
    explicit binary_numeric(binary_numeric const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit binary_numeric(binary_numeric&& other, ::takatori::util::object_creator creator);

    [[nodiscard]] binary_numeric* clone(::takatori::util::object_creator creator) const& override;
    [[nodiscard]] binary_numeric* clone(::takatori::util::object_creator creator) && override;

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

private:
    type_kind_type type_kind_;
    std::optional<precision_type> precision_;
};

} // namespace mizugaki::ast::type
