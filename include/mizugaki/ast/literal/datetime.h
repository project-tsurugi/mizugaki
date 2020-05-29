#pragma once

#include <mizugaki/ast/common/chars.h>
#include <mizugaki/ast/common/regioned.h>

#include "literal.h"

namespace mizugaki::ast::literal {

/**
 * @brief represents a datetime value.
 */
class datetime final : public literal {

    using super = literal;

public:
    /// @brief the value kind type.
    using value_kind_type = common::regioned<node_kind_type>;

    /// @brief the value type.
    using value_type = common::regioned<common::chars>;

    /// @brief the node kind of this.
    static constexpr kind_set tags {
            node_kind_type::date,
            node_kind_type::time,
            node_kind_type::timestamp,
    };

    /**
     * @brief creates a new instance.
     * @param value_kind the value kind, must be one of `date`, `time` or `timestamp`
     * @param value quoted string representation of the value
     * @param region the node region
     * @throws std::invalid_argument if kind is invalid
     * @see tags
     */
    explicit datetime(
            value_kind_type value_kind,
            value_type value,
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit datetime(datetime const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit datetime(datetime&& other, ::takatori::util::object_creator creator);

    [[nodiscard]] datetime* clone(::takatori::util::object_creator creator) const& override;
    [[nodiscard]] datetime* clone(::takatori::util::object_creator creator) && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the value kind.
     * @return the value kind
     */
    [[nodiscard]] value_kind_type& value_kind() noexcept;

    /// @copydoc value_kind()
    [[nodiscard]] value_kind_type const& value_kind() const noexcept;

    /**
     * @brief returns the quoted string representation of the value.
     * @return the value string
     */
    [[nodiscard]] value_type& value() noexcept;

    /// @copydoc value()
    [[nodiscard]] value_type const& value() const noexcept;

private:
    value_kind_type value_kind_;
    value_type value_;
};

} // namespace mizugaki::ast::literal
