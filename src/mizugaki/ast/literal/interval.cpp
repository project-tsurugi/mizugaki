#include <mizugaki/ast/literal/interval.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::literal {

using node_kind_type = literal::node_kind_type;
using value_type = interval::value_type;

using ::takatori::util::object_creator;

interval::interval(
        value_type value,
        region_type region) noexcept :
    super { region },
    value_ { std::move(value) }
{}

interval::interval(interval const& other, object_creator creator) :
    interval {
            value_type{other.value_, creator.allocator()},
            other.region(),
    }
{}

interval::interval(interval&& other, object_creator creator) :
    interval {
            value_type{std::move(other.value_), creator.allocator()},
            other.region(),
    }
{}

interval* interval::clone(object_creator creator) const& {
    return creator.create_object<interval>(*this, creator);
}

interval* interval::clone(object_creator creator) && {
    return creator.create_object<interval>(std::move(*this), creator);
}

node_kind_type interval::node_kind() const noexcept {
    return tag;
}

value_type& interval::value() noexcept {
    return value_;
}

value_type const& interval::value() const noexcept {
    return value_;
}

bool operator==(interval const& a, interval const& b) noexcept {
    return eq(a.value_, b.value_);
}

bool operator!=(interval const& a, interval const& b) noexcept {
    return !(a == b);
}

bool interval::equals(literal const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<interval>(other);
}

} // namespace mizugaki::ast::literal
