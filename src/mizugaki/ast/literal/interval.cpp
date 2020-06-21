#include <mizugaki/ast/literal/interval.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::literal {

using node_kind_type = literal::node_kind_type;
using value_type = interval::value_type;

using ::takatori::util::object_creator;

interval::interval(
        std::optional<sign_type> sign,
        value_type value,
        region_type region) noexcept :
    super { region },
    sign_ { std::move(sign) },
    value_ { std::move(value) }
{}

interval::interval(interval const& other, object_creator creator) :
    interval {
            other.sign_,
            value_type { other.value_, creator.allocator() },
            other.region(),
    }
{}

interval::interval(interval&& other, object_creator creator) :
    interval {
            std::move(other.sign_),
            value_type { std::move(other.value_), creator.allocator() },
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

std::optional<interval::sign_type>& interval::sign() noexcept {
    return sign_;
}

std::optional<interval::sign_type> const& interval::sign() const noexcept {
    return sign_;
}

value_type& interval::value() noexcept {
    return value_;
}

value_type const& interval::value() const noexcept {
    return value_;
}

bool operator==(interval const& a, interval const& b) noexcept {
    return eq(a.sign_, b.sign_)
            && eq(a.value_, b.value_);
}

bool operator!=(interval const& a, interval const& b) noexcept {
    return !(a == b);
}

bool interval::equals(literal const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void interval::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "sign"sv, sign_);
    property(acceptor, "value"sv, value_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, interval const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::literal
