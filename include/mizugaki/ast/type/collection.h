#pragma once

#include <optional>

#include <cstddef>

#include <mizugaki/ast/common/regioned.h>

#include <takatori/util/clone_tag.h>

#include "type.h"

namespace mizugaki::ast::type {

/**
 * @brief represents a collection type.
 */
class collection final : public type {

    using super = ast::type::type;

public:
    /// @brief the character length type.
    using length_type = common::regioned<std::size_t>;

    /// @brief the flexible length.
    static constexpr std::size_t flexible_length = static_cast<std::size_t>(-1);

    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::collection;

    /**
     * @brief creates a new instance.
     * @param element the element type
     * @param length the collection length, maybe flexible_length
     * @param region the node region
     * @throws std::invalid_argument if kind is invalid
     */
    explicit collection(
            std::unique_ptr<type> element,
            std::optional<length_type> length = {},
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param element the element type
     * @param length the collection length, maybe flexible_length
     * @param region the node region
     * @throws std::invalid_argument if kind is invalid
     * @attention this will take copy of arguments
     */
    explicit collection(
            type&& element,
            std::optional<length_type> length,
            region_type region = {});

    /**
     * @brief creates a new instance.
     * @tparam Args the types of lest length
     * @param element the element type
     * @param length the collection length, maybe flexible_length
     * @param args the rest of collection lengths, maybe flexible_length
     * @throws std::invalid_argument if kind is invalid
     * @attention this will take copy of arguments
     */
    template<class... Args>
    explicit collection(
            type&& element,
            std::optional<length_type> length,
            Args&&... args) :
        collection {
                collection {
                        std::move(element),
                        length,
                },
                std::forward<Args>(args)...
        }
    {}

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit collection(::takatori::util::clone_tag_t, collection const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit collection(::takatori::util::clone_tag_t, collection&& other);

    [[nodiscard]] collection* clone() const& override;
    [[nodiscard]] collection* clone() && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the element type.
     * @return the element type
     */
    [[nodiscard]] std::unique_ptr<ast::type::type>& element() noexcept;

    /// @copydoc element()
    [[nodiscard]] std::unique_ptr<ast::type::type> const& element() const noexcept;

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
    friend bool operator==(collection const& a, collection const& b) noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(collection const& a, collection const& b) noexcept;

protected:
    [[nodiscard]] bool equals(type const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    std::unique_ptr<type> element_;
    std::optional<length_type> length_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, collection const& value);

} // namespace mizugaki::ast::type
