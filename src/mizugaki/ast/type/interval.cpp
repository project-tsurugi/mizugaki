#include <mizugaki/ast/type/interval.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::type {

using node_kind_type = type::node_kind_type;

using ::takatori::util::object_creator;

interval::interval(region_type region) noexcept :
    super { region }
{}

interval::interval(interval const& other, object_creator) noexcept :
    interval { other.region() }
{}

interval::interval(interval&& other, object_creator) noexcept :
    interval { other.region() }
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

bool operator==(interval const& a, interval const& b) noexcept {
    (void) a;
    (void) b;
    return true;
}

bool operator!=(interval const& a, interval const& b) noexcept {
    return !(a == b);
}

bool interval::equals(type const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void interval::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, interval const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::type
