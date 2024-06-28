#pragma once

#include <vector>

#include <takatori/util/clone_tag.h>
#include <takatori/util/enum_tag.h>

#include <mizugaki/ast/common/chars.h>
#include <mizugaki/ast/common/vector.h>
#include <mizugaki/ast/common/regioned.h>

#include "literal.h"

namespace mizugaki::ast::literal {

/**
 * @brief represents a string value.
 */
class string final : public literal {

    using super = literal;

public:
    /// @brief the value kind type.
    using value_kind_type = common::regioned<node_kind_type>;

    /// @brief the value type.
    using value_type = common::regioned<common::chars>;

    /// @brief the concatenations type.
    using concatenations_type = std::vector<value_type>;

    /// @brief the quote character.
    static constexpr char quote_character = '\'';

    /// @brief the node kind of this.
    static constexpr kind_set tags {
            node_kind_type::character_string,
            node_kind_type::bit_string,
            node_kind_type::hex_string,
    };

    /**
     * @brief creates a new instance.
     * @param value_kind the value kind, must be one of `character_string`, `bit_string` or `hex_string`
     * @param value quoted string representation of the value (without value qualifier)
     * @param concatenations the rest quoted string values
     * @param region the node region
     * @throws std::invalid_argument if kind is invalid
     * @see tags
     */
    explicit string(
            value_kind_type value_kind,
            value_type value,
            concatenations_type concatenations = {},
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @tparam T the quoted string type
     * @tparam U list of quoted string types
     * @param value_kind the value kind, must be one of `character_string`, `bit_string` or `hex_string`
     * @param value quoted string representation of the value (without value qualifier)
     * @param concatenations the rest quoted string values
     * @throws std::invalid_argument if kind is invalid
     * @see tags
     */
    template<class T, class... U>
    explicit string(
            value_kind_type value_kind,
            T&& value,
            U&&... concatenations) :
        string {
                value_kind,
                value_type { common::chars { std::forward<T>(value) } },
                { value_type { common::chars { std::forward<U>(concatenations) } }... },
        }
    {}

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit string(::takatori::util::clone_tag_t, string const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit string(::takatori::util::clone_tag_t, string&& other);

    [[nodiscard]] string* clone() const& override;
    [[nodiscard]] string* clone() && override;

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
     * @attention This only provides only the first quoted element.
     *      To obtain the rest elements, also check concatenations().
     * @see concatenations()
     */
    [[nodiscard]] value_type& value() noexcept;

    /// @copydoc value()
    [[nodiscard]] value_type const& value() const noexcept;

    /**
     * @brief returns a sequence of quoted values except the first one.
     * @return the sequence of quoted values except the first one
     * @see value()
     */
    [[nodiscard]] concatenations_type& concatenations() noexcept;

    /// @copydoc concatenations()
    [[nodiscard]] concatenations_type const& concatenations() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(string const& a, string const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(string const& a, string const& b) noexcept;

protected:
    [[nodiscard]] bool equals(literal const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    value_kind_type value_kind_;
    value_type value_;
    concatenations_type concatenations_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, string const& value);

} // namespace mizugaki::ast::literal
