#include <mizugaki/ast/literal/boolean.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::literal {

using node_kind_type = literal::node_kind_type;
using value_type = boolean::value_type;


boolean::boolean(value_type value, region_type region) noexcept :
    super { region },
    value_ { value }
{}

boolean::boolean(::takatori::util::clone_tag_t, boolean const& other) noexcept :
    boolean {
            other.value_,
            other.region(),
    }
{}

boolean::boolean(::takatori::util::clone_tag_t, boolean&& other) noexcept :
    boolean {
            other.value_,
            other.region(),
    }
{}

boolean* boolean::clone() const& {
    return new boolean(::takatori::util::clone_tag, *this); // NOLINT
}

boolean* boolean::clone() && {
    return new boolean(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

node_kind_type boolean::node_kind() const noexcept {
    return tag;
}

value_type& boolean::value() noexcept {
    return value_;
}

value_type boolean::value() const noexcept {
    return value_;
}

bool operator==(boolean const& a, boolean const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return true;
    }
    return eq(a.value_, b.value_);
}

bool operator!=(boolean const& a, boolean const& b) noexcept {
    return !(a == b);
}

bool boolean::equals(literal const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void boolean::serialize(::takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "value"sv, value_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, boolean const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::literal
