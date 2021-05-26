#include <mizugaki/ast/type/simple.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

#include "utils.h"

namespace mizugaki::ast::type {

using node_kind_type = type::node_kind_type;
using type_kind_type = simple::type_kind_type;


simple::simple(type_kind_type type_kind, region_type region) :
    super { region },
    type_kind_ { type_kind }
{
    utils::validate_kind(tags, type_kind);
}

simple::simple(::takatori::util::clone_tag_t, simple const& other) :
    simple {
            other.type_kind_,
            other.region(),
    }
{}

simple::simple(::takatori::util::clone_tag_t, simple&& other) :
    simple {
            other.type_kind_,
            other.region(),
    }
{}

simple* simple::clone() const& {
    return new simple(::takatori::util::clone_tag, *this); // NOLINT
}

simple* simple::clone() && {
    return new simple(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

node_kind_type simple::node_kind() const noexcept {
    return type_kind_;
}

bool operator==(simple const& a, simple const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return true;
    }
    return eq(a.type_kind_, b.type_kind_);
}

bool operator!=(simple const& a, simple const& b) noexcept {
    return !(a == b);
}

bool simple::equals(type const& other) const noexcept {
    return tags.contains(other.node_kind())
            && *this == unsafe_downcast<simple>(other);
}

void simple::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, simple const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::type
