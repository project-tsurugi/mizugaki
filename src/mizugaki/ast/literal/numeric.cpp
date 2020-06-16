#include <mizugaki/ast/literal/numeric.h>

#include <mizugaki/ast/compare_utils.h>

#include "utils.h"

namespace mizugaki::ast::literal {

using node_kind_type = literal::node_kind_type;
using sign_type = numeric::sign_type;
using value_type = numeric::value_type;

using ::takatori::util::object_creator;

numeric::numeric(
        value_kind_type value_kind,
        std::optional<sign_type> sign,
        value_type unsigned_value,
        region_type region) :
    super { region },
    value_kind_ { value_kind },
    sign_ { std::move(sign) },
    unsigned_value_ { std::move(unsigned_value) }
{
    utils::validate_kind(tags, value_kind);
}

numeric::numeric(numeric const& other, object_creator creator) :
    numeric {
            other.value_kind_,
            other.sign_,
            value_type{other.unsigned_value_, creator.allocator()},
            other.region(),
    }
{}

numeric::numeric(numeric&& other, object_creator creator) :
    numeric {
            other.value_kind_,
            std::move(other.sign_),
            value_type { std::move(other.unsigned_value_), creator.allocator() },
            other.region(),
    }
{}

numeric* numeric::clone(object_creator creator) const& {
    return creator.create_object<numeric>(*this, creator);
}

numeric* numeric::clone(object_creator creator) && {
    return creator.create_object<numeric>(std::move(*this), creator);
}

node_kind_type numeric::node_kind() const noexcept {
    return value_kind_;
}

numeric::value_kind_type& numeric::value_kind() noexcept {
    return value_kind_;
}

numeric::value_kind_type const& numeric::value_kind() const noexcept {
    return value_kind_;
}

std::optional<sign_type>& numeric::sign() noexcept {
    return sign_;
}

std::optional<sign_type> const& numeric::sign() const noexcept {
    return sign_;
}

value_type& numeric::unsigned_value() noexcept {
    return unsigned_value_;
}

value_type const& numeric::unsigned_value() const noexcept {
    return unsigned_value_;
}

bool operator==(numeric const& a, numeric const& b) noexcept {
    return eq(a.value_kind_, b.value_kind_)
            && eq(a.unsigned_value_, b.unsigned_value_);
}

bool operator!=(numeric const& a, numeric const& b) noexcept {
    return !(a == b);
}

bool numeric::equals(literal const& other) const noexcept {
    return tags.contains(other.node_kind())
            && *this == unsafe_downcast<numeric>(other);
}

} // namespace mizugaki::ast::literal
