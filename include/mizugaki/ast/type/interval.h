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
    static constexpr node_kind_type tag = node_kind_type::interval;

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

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(interval const& a, interval const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(interval const& a, interval const& b) noexcept;

protected:
    [[nodiscard]] bool equals(type const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, interval const& value);

} // namespace mizugaki::ast::type
