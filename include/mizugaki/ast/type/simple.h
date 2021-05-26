#pragma once

#include <takatori/util/clone_tag.h>

#include "type.h"

namespace mizugaki::ast::type {

/**
 * @brief represents a simple type which only consists of its type kind.
 */
class simple final : public type {

    using super = type;

public:
    /// @brief the simple kind type.
    using type_kind_type = node_kind_type;

    /// @brief the available node kind of this.
    static constexpr kind_set tags {
            node_kind_type::unknown,
            node_kind_type::tiny_integer,
            node_kind_type::small_integer,
            node_kind_type::integer,
            node_kind_type::big_integer,
            node_kind_type::float_,
            node_kind_type::real,
            node_kind_type::double_precision,
            node_kind_type::boolean,
            node_kind_type::date,
    };

    /**
     * @brief creates a new instance.
     * @param type_kind the simple type kind
     * @param region the node region
     * @throws std::invalid_argument if kind is invalid
     * @see tags
     */
    explicit simple(type_kind_type type_kind, region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit simple(::takatori::util::clone_tag_t, simple const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit simple(::takatori::util::clone_tag_t, simple&& other);

    [[nodiscard]] simple* clone() const& override;
    [[nodiscard]] simple* clone() && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(simple const& a, simple const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(simple const& a, simple const& b) noexcept;

protected:
    [[nodiscard]] bool equals(type const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    type_kind_type type_kind_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, simple const& value);

} // namespace mizugaki::ast::type
