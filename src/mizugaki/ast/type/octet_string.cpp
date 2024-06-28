#include <mizugaki/ast/type/octet_string.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

#include "utils.h"

namespace mizugaki::ast::type {

using node_kind_type = type::node_kind_type;
using type_kind_type = octet_string::type_kind_type;
using length_type = octet_string::length_type;


octet_string::octet_string(
        type_kind_type type_kind,
        std::optional<length_type> length,
        region_type region) :
    super { region },
    type_kind_ { type_kind },
    length_ { length }
{
    utils::validate_kind(tags, *type_kind);
}

octet_string::octet_string(::takatori::util::clone_tag_t, octet_string const& other) :
    octet_string {
            other.type_kind_,
            other.length_,
            other.region(),
    }
{}

octet_string::octet_string(::takatori::util::clone_tag_t, octet_string&& other) :
    octet_string {
            other.type_kind_,
            other.length_,
            other.region(),
    }
{}

octet_string* octet_string::clone() const& {
    return new octet_string(::takatori::util::clone_tag, *this); // NOLINT
}

octet_string* octet_string::clone() && {
    return new octet_string(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

node_kind_type octet_string::node_kind() const noexcept {
    return *type_kind_;
}

type_kind_type& octet_string::type_kind() noexcept {
    return type_kind_;
}

type_kind_type const& octet_string::type_kind() const noexcept {
    return type_kind_;
}

bool octet_string::is_varying() const noexcept {
    return node_kind() == node_kind_type::octet_varying;
}

bool octet_string::is_flexible_length() const noexcept {
    return length_ == flexible_length;
}

std::optional<length_type>& octet_string::length() noexcept {
    return length_;
}

std::optional<length_type> const& octet_string::length() const noexcept {
    return length_;
}

bool operator==(octet_string const& a, octet_string const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return true;
    }
    return eq(a.type_kind_, b.type_kind_)
        && eq(a.length_, b.length_);
}

bool operator!=(octet_string const& a, octet_string const& b) noexcept {
    return !(a == b);
}

bool octet_string::equals(type const& other) const noexcept {
    return tags.contains(other.node_kind())
        && *this == unsafe_downcast<octet_string>(other);
}

void octet_string::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "type_kind"sv, type_kind_);
    property(acceptor, "length"sv, length_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, octet_string const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::type
