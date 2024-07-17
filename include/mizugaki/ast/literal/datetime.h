#pragma once

#include <takatori/util/clone_tag.h>

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
            node_kind_type::time_with_time_zone,
            node_kind_type::timestamp,
            node_kind_type::timestamp_with_time_zone,
    };

    /**
     * @brief creates a new instance.
     * @param value_kind the value kind, must be one of `date`, `time`, timestamp`, or their `*_with_time_zone`
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
     * @tparam T the quoted string type
     * @param value_kind the value kind, must be one of `date`, `time` or `timestamp`
     * @param value quoted string representation of the value
     * @param region the node region
     * @throws std::invalid_argument if kind is invalid
     * @see tags
     */
    template<class T>
    explicit datetime(
            value_kind_type value_kind,
            T&& value,
            region_type region = {}) :
        datetime {
                value_kind,
                value_type { common::chars { std::forward<T>(value) } },
                region,
        }
    {}

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
    [[nodiscard]] bool equals(literal const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    value_kind_type value_kind_;
    value_type value_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, datetime const& value);

} // namespace mizugaki::ast::literal
