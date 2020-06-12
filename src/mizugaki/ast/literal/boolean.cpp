#include <mizugaki/ast/literal/boolean.h>

namespace mizugaki::ast::literal {

using node_kind_type = literal::node_kind_type;
using value_type = boolean::value_type;

using ::takatori::util::object_creator;

boolean::boolean(value_type value, region_type region) noexcept :
    super { region },
    value_ { value }
{}

boolean::boolean(boolean const& other, object_creator) noexcept :
    boolean {
            other.value_,
            other.region(),
    }
{}

boolean::boolean(boolean&& other, object_creator) noexcept :
    boolean {
            other.value_,
            other.region(),
    }
{}

boolean* boolean::clone(object_creator creator) const& {
    return creator.create_object<boolean>(*this, creator);
}

boolean* boolean::clone(object_creator creator) && {
    return creator.create_object<boolean>(std::move(*this), creator);
}

node_kind_type boolean::node_kind() const noexcept {
    return tag;
}

value_type& boolean::value() noexcept {
    return value_;
}

value_type boolean::value() const noexcept {
    return value_;
}

} // namespace mizugaki::ast::literal
