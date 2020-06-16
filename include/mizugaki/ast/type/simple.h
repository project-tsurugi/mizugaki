#pragma once

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
     * @param creator the object creator
     */
    explicit simple(simple const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit simple(simple&& other, ::takatori::util::object_creator creator);

    [[nodiscard]] simple* clone(::takatori::util::object_creator creator) const& override;
    [[nodiscard]] simple* clone(::takatori::util::object_creator creator) && override;

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

private:
    type_kind_type type_kind_;
};

} // namespace mizugaki::ast::type
