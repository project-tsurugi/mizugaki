#include <mizugaki/ast/type/datetime.h>

#include "utils.h"

namespace mizugaki::ast::type {

using node_kind_type = type::node_kind_type;
using type_kind_type = datetime::type_kind_type;
using enable_type = datetime::enable_type;

using ::takatori::util::object_creator;

datetime::datetime(
        type_kind_type type_kind,
        std::optional<enable_type> has_time_zone,
        region_type region) :
    super { region },
    type_kind_ { type_kind },
    has_time_zone_ { std::move(has_time_zone) }
{
    utils::validate_kind(tags, *type_kind);
}

datetime::datetime(datetime const& other, object_creator) :
    datetime {
            other.type_kind_,
            other.has_time_zone_,
            other.region(),
    }
{}

datetime::datetime(datetime&& other, object_creator) :
    datetime {
            other.type_kind_,
            std::move(other.has_time_zone_),
            other.region(),
    }
{}

datetime* datetime::clone(object_creator creator) const& {
    return creator.create_object<datetime>(*this, creator);
}

datetime* datetime::clone(object_creator creator) && {
    return creator.create_object<datetime>(std::move(*this), creator);
}

node_kind_type datetime::node_kind() const noexcept {
    return *type_kind_;
}

type_kind_type& datetime::type_kind() noexcept {
    return type_kind_;
}

type_kind_type const& datetime::type_kind() const noexcept {
    return type_kind_;
}

std::optional<enable_type>& datetime::has_time_zone() noexcept {
    return has_time_zone_;
}

std::optional<enable_type> const& datetime::has_time_zone() const noexcept {
    return has_time_zone_;
}

} // namespace mizugaki::ast::type
