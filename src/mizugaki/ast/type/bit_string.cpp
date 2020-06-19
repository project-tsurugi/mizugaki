#include <mizugaki/ast/type/bit_string.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

#include "utils.h"

namespace mizugaki::ast::type {

using node_kind_type = type::node_kind_type;
using type_kind_type = bit_string::type_kind_type;
using length_type = bit_string::length_type;

using ::takatori::util::object_creator;

bit_string::bit_string(
        type_kind_type type_kind,
        std::optional<length_type> length,
        region_type region) :
    super { region },
    type_kind_ { type_kind },
    length_ { std::move(length) }
{
    utils::validate_kind(tags, *type_kind);
}

bit_string::bit_string(bit_string const& other, object_creator) :
    bit_string {
            other.type_kind_,
            other.length_,
            other.region(),
    }
{}

bit_string::bit_string(bit_string&& other, object_creator) :
    bit_string {
            other.type_kind_,
            std::move(other.length_),
            other.region(),
    }
{}

bit_string* bit_string::clone(object_creator creator) const& {
    return creator.create_object<bit_string>(*this, creator);
}

bit_string* bit_string::clone(object_creator creator) && {
    return creator.create_object<bit_string>(std::move(*this), creator);
}

node_kind_type bit_string::node_kind() const noexcept {
    return *type_kind_;
}

type_kind_type& bit_string::type_kind() noexcept {
    return type_kind_;
}

type_kind_type const& bit_string::type_kind() const noexcept {
    return type_kind_;
}

bool bit_string::is_flexible_length() const noexcept {
    return length_ == flexible_length;
}

std::optional<length_type>& bit_string::length() noexcept {
    return length_;
}

std::optional<length_type> const& bit_string::length() const noexcept {
    return length_;
}

bool operator==(bit_string const& a, bit_string const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return true;
    }
    return eq(a.type_kind_, b.type_kind_)
            && eq(a.length_, b.length_);
}

bool operator!=(bit_string const& a, bit_string const& b) noexcept {
    return !(a == b);
}

bool bit_string::equals(type const& other) const noexcept {
    return tags.contains(other.node_kind())
            && *this == unsafe_downcast<bit_string>(other);
}

void bit_string::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "type_kind"sv, type_kind_);
    property(acceptor, "length"sv, length_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, bit_string const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::type
