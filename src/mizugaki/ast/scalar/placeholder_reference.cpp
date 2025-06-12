#include <mizugaki/ast/scalar/placeholder_reference.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::scalar {

placeholder_reference::placeholder_reference(
        index_type index,
        region_type region) noexcept:
    super { region },
    index_ { index }
{}

placeholder_reference::placeholder_reference(::takatori::util::clone_tag_t, placeholder_reference const& other) :
    placeholder_reference {
            other.index_,
            other.region(),
    }
{}

placeholder_reference::placeholder_reference(::takatori::util::clone_tag_t, placeholder_reference&& other) :
    placeholder_reference {
            other.index_,
            other.region(),
    }
{}

placeholder_reference* placeholder_reference::clone() const& {
    return new placeholder_reference(::takatori::util::clone_tag, *this); // NOLINT
}

placeholder_reference* placeholder_reference::clone() && {
    return new placeholder_reference(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

expression::node_kind_type placeholder_reference::node_kind() const noexcept {
    return tag;
}

placeholder_reference::index_type& placeholder_reference::index() noexcept {
    return index_;
}

placeholder_reference::index_type const& placeholder_reference::index() const noexcept {
    return index_;
}

bool operator==(placeholder_reference const& a, placeholder_reference const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return true;
    }
    return a.index_ == b.index_;
}

bool operator!=(placeholder_reference const& a, placeholder_reference const& b) noexcept {
    return !(a == b);
}

bool placeholder_reference::equals(expression const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void placeholder_reference::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "index"sv, index_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, placeholder_reference const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::scalar
