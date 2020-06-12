#include <mizugaki/ast/type/character_string.h>

#include "utils.h"

namespace mizugaki::ast::type {

using node_kind_type = type::node_kind_type;
using type_kind_type = character_string::type_kind_type;
using length_type = character_string::length_type;

using ::takatori::util::object_creator;

character_string::character_string(
        type_kind_type type_kind,
        std::optional<length_type> length,
        region_type region) :
    super { region },
    type_kind_ { type_kind },
    length_ { std::move(length) }
{
    utils::validate_kind(tags, *type_kind);
}

character_string::character_string(character_string const& other, object_creator) :
    character_string {
            other.type_kind_,
            other.length_,
            other.region(),
    }
{}

character_string::character_string(character_string&& other, object_creator) :
    character_string {
            other.type_kind_,
            std::move(other.length_),
            other.region(),
    }
{}

character_string* character_string::clone(object_creator creator) const& {
    return creator.create_object<character_string>(*this, creator);
}

character_string* character_string::clone(object_creator creator) && {
    return creator.create_object<character_string>(std::move(*this), creator);
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

bool character_string::is_flexible_length() const noexcept {
    return length_ == flexible_length;
}

std::optional<length_type>& character_string::length() noexcept {
    return length_;
}

std::optional<length_type> const& character_string::length() const noexcept {
    return length_;
}

} // namespace mizugaki::ast::type
