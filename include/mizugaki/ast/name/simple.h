#pragma once

#include <takatori/util/clone_tag.h>

#include "name.h"

namespace mizugaki::ast::name {

/**
 * @brief represents a simple name.
 */
class simple final : public name {

    using super = name;

public:
    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::simple;

    /**
     * @brief creates a new instance.
     * @param identifier the name identifier
     * @param region the node region
     */
    explicit simple(identifier_type identifier, region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @tparam T the identifier source type
     * @param identifier the name identifier
     */
    template<class T>
    explicit simple(T const& identifier) noexcept(std::is_nothrow_constructible_v<identifier_type, T const&>)
        : identifier_ { identifier }
    {}

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

    /// @copydoc identifier()
    [[nodiscard]] identifier_type const& last_identifier() const noexcept override;

    /**
     * @brief always returns empty.
     * @return empty
     */
    [[nodiscard]] ::takatori::util::optional_ptr<name const> optional_qualifier() const noexcept override;

    /**
     * @brief returns the identifier of this name.
     * @return the identifier
     */
    [[nodiscard]] identifier_type& identifier() noexcept;

    /// @copydoc identifier()
    [[nodiscard]] identifier_type const& identifier() const noexcept;

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
    [[nodiscard]] bool equals(name const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    identifier_type identifier_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, simple const& value);

} // namespace mizugaki::ast::name
