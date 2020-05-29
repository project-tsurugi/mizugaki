#include <mizugaki/ast/type/bit_string.h>

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
    bit_string(
            other.type_kind_,
            other.length_,
            other.region())
{}

bit_string::bit_string(bit_string&& other, object_creator) :
    bit_string(
            other.type_kind_,
            std::move(other.length_),
            other.region())
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

} // namespace mizugaki::ast::type
