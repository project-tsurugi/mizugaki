#include <mizugaki/ast/type/character_string.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

#include "utils.h"

namespace mizugaki::ast::type {

using node_kind_type = type::node_kind_type;
using type_kind_type = character_string::type_kind_type;
using length_type = character_string::length_type;


character_string::character_string(
        type_kind_type type_kind,
        std::optional<length_type> length,
        region_type region) :
    super { region },
    type_kind_ { type_kind },
    length_ { length }
{
    utils::validate_kind(tags, *type_kind);
}

character_string::character_string(::takatori::util::clone_tag_t, character_string const& other) :
    character_string {
            other.type_kind_,
            other.length_,
            other.region(),
    }
{}

character_string::character_string(::takatori::util::clone_tag_t, character_string&& other) :
    character_string {
            other.type_kind_,
            other.length_,
            other.region(),
    }
{}

character_string* character_string::clone() const& {
    return new character_string(::takatori::util::clone_tag, *this); // NOLINT
}

character_string* character_string::clone() && {
    return new character_string(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

node_kind_type character_string::node_kind() const noexcept {
    return *type_kind_;
}

type_kind_type& character_string::type_kind() noexcept {
    return type_kind_;
}

type_kind_type const& character_string::type_kind() const noexcept {
    return type_kind_;
}

bool character_string::is_varying() const noexcept {
    return node_kind() == node_kind_type::character_varying;
}

bool character_string::is_flexible_length() const noexcept {
    return length_ == flexible_length;
}

std::optional<length_type>& character_string::length() noexcept {
    return length_;
}

std::optional<length_type> const& character_string::length() const noexcept {
    return length_;
}

bool operator==(character_string const& a, character_string const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return true;
    }
    return eq(a.type_kind_, b.type_kind_)
            && eq(a.length_, b.length_);
}

bool operator!=(character_string const& a, character_string const& b) noexcept {
    return !(a == b);
}

bool character_string::equals(type const& other) const noexcept {
    return tags.contains(other.node_kind())
            && *this == unsafe_downcast<character_string>(other);
}

void character_string::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "length"sv, length_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, character_string const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::type
