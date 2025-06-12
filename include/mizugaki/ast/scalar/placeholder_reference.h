#pragma once

#include <takatori/util/clone_tag.h>

#include "expression.h"

namespace mizugaki::ast::scalar {

/**
 * @brief refers placeholders.
 */
class placeholder_reference final : public expression {

    using super = expression;

public:
    /// @brief the node kind of this.
    static constexpr node_kind_type tag = node_kind_type::placeholder_reference;

    /// @brief the placeholder index type.
    using index_type = std::size_t;

    /**
     * @brief creates a new instance.
     * @param index the placeholder index (1-origin)
     * @param region the node region
     */
    explicit placeholder_reference(
            index_type index,
            region_type region = {}) noexcept;

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit placeholder_reference(::takatori::util::clone_tag_t, placeholder_reference const& other);

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit placeholder_reference(::takatori::util::clone_tag_t, placeholder_reference&& other);

    [[nodiscard]] placeholder_reference* clone() const& override;
    [[nodiscard]] placeholder_reference* clone() && override;

    [[nodiscard]] node_kind_type node_kind() const noexcept override;

    /**
     * @brief returns the placeholder index.
     * @return the placeholder index
     */
    [[nodiscard]] index_type& index() noexcept;

    /// @brief index()
    [[nodiscard]] index_type const& index() const noexcept;

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(placeholder_reference const& a, placeholder_reference const& b) noexcept;
    
    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(placeholder_reference const& a, placeholder_reference const& b) noexcept;
    
protected:
    [[nodiscard]] bool equals(expression const& other) const noexcept override;
    void serialize(::takatori::serializer::object_acceptor& acceptor) const override;

private:
    index_type index_;
};

/**
 * @brief appends string representation of the given value.
 * @param out the target output
 * @param value the target value
 * @return the output
 */
std::ostream& operator<<(std::ostream& out, placeholder_reference const& value);

} // namespace mizugaki::ast::scalar
