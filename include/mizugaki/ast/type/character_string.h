#pragma once

#include <optional>

#include <cstddef>

#include <mizugaki/ast/common/regioned.h>

#include "type.h"

namespace mizugaki::ast::type {

/**
 * @brief represents a character string type.
 */
class character_string final : public type {

    using super = type;

public:
    /// @brief the type kind type.
    using type_kind_type = common::regioned<node_kind_type>;

    /// @brief the character length type.
    using length_type = common::regioned<std::size_t>;

    /// @brief the flexible length.
    static constexpr std::size_t flexible_length = static_cast<std::size_t>(-1);

    /// @brief the available node kind of this.
    static constexpr kind_set tags {
            node_kind_type::character,
            node_kind_type::character_varying,
    };

    /**
     * @brief creates a new instance.
     * @param type_kind the character_string type kind
     * @param length the string length (in bytes), maybe flexible_length
     * @param region the node region
     * @throws std::invalid_argument if kind is invalid
     * @see tags
     */
    explicit character_string(
            type_kind_type type_kind,
            std::optional<length_type> length,
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @param other the copy source
     * @param creator the object creator
     */
    explicit character_string(character_string const& other, ::takatori::util::object_creator creator);

    /**
     * @brief creates a new instance.
     * @param other the move source
     * @param creator the object creator
     */
    explicit character_string(character_string&& other, ::takatori::util::object_creator creator);

    [[nodiscard]] character_string* clone(::takatori::util::object_creator creator) const& override;
    [[nodiscard]] character_string* clone(::takatori::util::object_creator creator) && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the type kind.
     * @return the type kind
     */
    [[nodiscard]] type_kind_type& type_kind() noexcept;

    /// @copydoc type_kind()
    [[nodiscard]] type_kind_type const& type_kind() const noexcept;

    /**
     * @brief returns whether or not this type is flexible length.
     * @return true if this is flexible length
     * @return false otherwise
     * @see flexible_length
     */
    [[nodiscard]] bool is_flexible_length() const noexcept;

    /**
     * @brief returns the character string length.
     * @return the available number of bytes in the string
     * @return empty if the string length is not defined
     * @return flexible_length if this character string has flexible length
     */
    [[nodiscard]] std::optional<length_type>& length() noexcept;

    /// @copydoc length()
    [[nodiscard]] std::optional<length_type> const& length() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(character_string const& a, character_string const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(character_string const& a, character_string const& b) noexcept;

protected:
    [[nodiscard]] bool equals(type const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    type_kind_type type_kind_;
    std::optional<length_type> length_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, character_string const& value);

} // namespace mizugaki::ast::type
