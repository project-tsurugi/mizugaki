#include <mizugaki/ast/literal/datetime.h>

#include "utils.h"

namespace mizugaki::ast::literal {

using node_kind_type = literal::node_kind_type;
using value_type = datetime::value_type;

using ::takatori::util::object_creator;

datetime::datetime(
        value_kind_type value_kind,
        value_type value,
        region_type region) :
    super { region },
    value_kind_ { value_kind },
    value_ { std::move(value) }
{
    utils::validate_kind(tags, *value_kind);
}

datetime::datetime(datetime const& other, object_creator creator) :
    datetime {
            other.value_kind_,
            value_type{other.value_, creator.allocator()},
            other.region(),
    }
{}

datetime::datetime(datetime&& other, object_creator creator) :
    datetime {
            other.value_kind_,
            value_type{std::move(other.value_), creator.allocator()},
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
    return *value_kind_;
}

datetime::value_kind_type& datetime::value_kind() noexcept {
    return value_kind_;
}

datetime::value_kind_type const& datetime::value_kind() const noexcept {
    return value_kind_;
}

value_type& datetime::value() noexcept {
    return value_;
}

value_type const& datetime::value() const noexcept {
    return value_;
}

} // namespace mizugaki::ast::literal
