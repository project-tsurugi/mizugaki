#pragma once

#include "type.h"

namespace mizugaki::ast::type {

/**
 * @brief represents a datetime interval type.
 */
class interval final : public type {

    using super = type;

public:
    // FIXME: impl using qualifier_type = common::interval_qualifier;

    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::boolean;

    /**
     * @brief creates a new instance.
     * @param region the node region
     * @see tags
     */
    explicit interval(
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit interval(interval const& other, ::takatori::util::object_creator creator) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit interval(interval&& other, ::takatori::util::object_creator creator) noexcept;

    [[nodiscard]] interval* clone(::takatori::util::object_creator creator) const& override;
    [[nodiscard]] interval* clone(::takatori::util::object_creator creator) && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    // FIXME: impl std::optional<qualifier_type> start()
    // FIXME: impl std::optional<qualifier_type> end()

private:
};

} // namespace mizugaki::ast::type
