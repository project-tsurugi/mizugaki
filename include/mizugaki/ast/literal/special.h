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
        return creator.create_unique<special>(*this, region());
    }

    [[nodiscard]] special* clone(::takatori::util::object_creator creator) && override {
        return creator.create_unique<special>(std::move(*this), region());
    }

    [[nodiscard]] node_kind_type node_kind() const noexcept override {
        return tag;
    }
};

} // namespace mizugaki::ast::literal
