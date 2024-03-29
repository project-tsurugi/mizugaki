#pragma once

#include <optional>

#include <mizugaki/ast/common/regioned.h>

#include <takatori/util/clone_tag.h>

#include "type.h"

namespace mizugaki::ast::type {

/**
 * @brief represents a datetime type which can have time zone.
 * @note date is represented by simple.
 */
class datetime final : public type {

    using super = type;

public:
    /// @brief the type kind type.
    using type_kind_type = common::regioned<node_kind_type>;

    /// @brief truth type with element region information.
    using bool_type = common::regioned<bool>;

    /// @brief the available node kind of this.
    static constexpr kind_set tags {
            node_kind_type::time,
            node_kind_type::timestamp,
    };

    /**
     * @brief creates a new instance.
     * @param type_kind the datetime type kind
     * @param has_time_zone whether or not time zone is enabled
     * @param region the node region
     * @throws std::invalid_argument if kind is invalid
     * @see tags
     */
    explicit datetime(
            type_kind_type type_kind,
            std::optional<bool_type> has_time_zone = {},
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit datetime(::takatori::util::clone_tag_t, datetime const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit datetime(::takatori::util::clone_tag_t, datetime&& other);

    [[nodiscard]] datetime* clone() const& override;
    [[nodiscard]] datetime* clone() && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the type kind.
     * @return the type kind
     */
    [[nodiscard]] type_kind_type& type_kind() noexcept;

    /// @copydoc type_kind()
    [[nodiscard]] type_kind_type const& type_kind() const noexcept;

    /**
     * @brief returns whether or not this type uses time zone.
     * @return true this type uses time zone
     * @return false this type does not use time zone
     * @return empty if time zone information is not provided
     */
    [[nodiscard]] std::optional<bool_type>& has_time_zone() noexcept;

    /// @copydoc has_time_zone()
    [[nodiscard]] std::optional<bool_type> const& has_time_zone() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(datetime const& a, datetime const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(datetime const& a, datetime const& b) noexcept;

protected:
    [[nodiscard]] bool equals(type const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    type_kind_type type_kind_;
    std::optional<bool_type> has_time_zone_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, datetime const& value);

} // namespace mizugaki::ast::type
