#include <mizugaki/ast/literal/interval.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::literal {

using node_kind_type = literal::node_kind_type;
using value_type = interval::value_type;


interval::interval(
        std::optional<sign_type> sign,
        value_type value,
        region_type region) noexcept :
    super { region },
    sign_ { sign },
    value_ { std::move(value) }
{}

interval::interval(::takatori::util::clone_tag_t, interval const& other) :
    interval {
            other.sign_,
            value_type { other.value_ },
            other.region(),
    }
{}

interval::interval(::takatori::util::clone_tag_t, interval&& other) :
    interval {
            other.sign_,
            value_type { std::move(other.value_) },
            other.region(),
    }
{}

interval* interval::clone() const& {
    return new interval(::takatori::util::clone_tag, *this); // NOLINT
}

interval* interval::clone() && {
    return new interval(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
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
