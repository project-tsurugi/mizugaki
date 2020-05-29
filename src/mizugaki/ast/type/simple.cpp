#include <mizugaki/ast/type/simple.h>

#include "utils.h"

namespace mizugaki::ast::type {

using node_kind_type = type::node_kind_type;
using type_kind_type = simple::type_kind_type;

using ::takatori::util::object_creator;

simple::simple(type_kind_type type_kind, region_type region) :
    super { region },
    type_kind_ { type_kind }
{
    utils::validate_kind(tags, type_kind);
}

simple::simple(simple const& other, object_creator) :
    simple(other.type_kind_, other.region())
{}

simple::simple(simple&& other, object_creator) :
    simple(other.type_kind_, other.region())
{}

simple* simple::clone(object_creator creator) const& {
    return creator.create_object<simple>(*this, creator);
}

simple* simple::clone(object_creator creator) && {
    return creator.create_object<simple>(std::move(*this), creator);
}

node_kind_type simple::node_kind() const noexcept {
    return type_kind_;
}

} // namespace mizugaki::ast::type
