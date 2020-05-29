#pragma once

#include <optional>

#include <cstddef>

#include <mizugaki/ast/common/regioned.h>

#include "type.h"

namespace mizugaki::ast::type {

/**
 * @brief represents a decimal numeric type.
 */
class decimal final : public type {

    using super = type;

public:
    /// @brief the type kind type.
    using type_kind_type = common::regioned<node_kind_type>;

    /// @brief the precision type.
    using precision_type = std::size_t;

    /// @brief the scale type.
    using scale_type = std::size_t;

    /// @brief the flexible precision.
    static constexpr precision_type flexible_precision = static_cast<precision_type>(-1);

    /// @brief the available node kind of this.
    static constexpr kind_set tags {
            node_kind_type::numeric,
            node_kind_type::decimal,
    };

    /**
     * @brief creates a new instance.
     * @param type_kind the decimal type kind
     * @param precision the guaranteed number of decimal significands
     * @param scale the available number of digits in fractional part
     * @param region the node region
     * @throws std::invalid_argument if kind is invalid
     * @see tags
     */
    explicit decimal(
            type_kind_type type_kind,
            std::optional<precision_type> precision,
            std::optional<scale_type> scale,
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit decimal(decimal const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit decimal(decimal&& other, ::takatori::util::object_creator creator);

    [[nodiscard]] decimal* clone(::takatori::util::object_creator creator) const& override;
    [[nodiscard]] decimal* clone(::takatori::util::object_creator creator) && override;

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
     * @brief returns the guaranteed number of decimal significands.
     * @return the available number of digits in the decimal
     * @return empty if the precision is not defined
     * @return flexible_precision if this decimal has flexible precision
     */
    [[nodiscard]] std::optional<precision_type>& precision() noexcept;

    /// @copydoc precision()
    [[nodiscard]] std::optional<precision_type> const& precision() const noexcept;

    /**
     * @brief returns the available number of digits in the fractional part.
     * @return the available number of digits in the decimal
     * @return empty if the scale is not defined
     */
    [[nodiscard]] std::optional<scale_type>& scale() noexcept;

    /// @copydoc scale()
    [[nodiscard]] std::optional<scale_type> const& scale() const noexcept;

private:
    type_kind_type type_kind_;
    std::optional<precision_type> precision_;
    std::optional<scale_type> scale_;
};

} // namespace mizugaki::ast::type
