#pragma once

#include <mizugaki/ast/common/regioned.h>

#include "literal.h"

namespace mizugaki::ast::literal {

/**
 * @brief represents a special value.
 */
template<kind Kind>
class special final : public literal {

    using super = literal;

public:
    /// @brief the node kind of this.
    static constexpr node_kind_type tag = Kind;

    /// @brief available kinds.
    static constexpr kind_set tags {
            kind::null,
            kind::empty,
            kind::default_,
    };

    static_assert(tags.contains(tag));

    /**
     * @brief creates a new instance.
     * @param region the node region
     */
    explicit constexpr special(region_type region = {}) noexcept :
        super { region }
    {}

    /**
     * @brief creates a new instance.
     * @param other the copy source
     */
    explicit constexpr special(special const& other, ::takatori::util::object_creator) noexcept
        : super(other.region())
    {}

    /**
     * @brief creates a new instance.
     * @param other the move source
     */
    explicit constexpr special(special&& other, ::takatori::util::object_creator) noexcept
        : super(other.region())
    {}

    [[nodiscard]] special* clone(::takatori::util::object_creator creator) const& override {
        return creator.create_object<special>(*this, creator);
    }

    [[nodiscard]] special* clone(::takatori::util::object_creator creator) && override {
        return creator.create_object<special>(std::move(*this), creator);
    }

    [[nodiscard]] node_kind_type node_kind() const noexcept override {
        return tag;
    }

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are equivalent
     * @return false otherwise
     */
    friend bool operator==(special const& a, special const& b) noexcept {
        (void) a;
        (void) b;
        return true;
    }

    /**
     * @brief compares two values.
     * @param a the first value
     * @param b the second value
     * @return true if the both are different
     * @return false otherwise
     */
    friend bool operator!=(special const& a, special const& b) noexcept {
        return !(a == b);
    }

    /**
     * @brief appends string representation of the given value.
     * @param out the target output
     * @param value the target value
     * @return the output
     */
    friend std::ostream& operator<<(std::ostream& out, special const& value) {
        return out << static_cast<literal const&>(value);
    }

protected:
    [[nodiscard]] bool equals(literal const& other) const noexcept override {
        return other.node_kind() == tag;
    }

    using literal::serialize;
};

} // namespace mizugaki::ast::literal
