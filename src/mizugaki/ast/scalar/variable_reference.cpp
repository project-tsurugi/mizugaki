#include <mizugaki/ast/scalar/variable_reference.h>

#include <takatori/util/clonable.h>

namespace mizugaki::ast::scalar {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

variable_reference::variable_reference(
        unique_object_ptr<name::name> name,
        region_type region) noexcept:
    super { region },
    name_ { std::move(name) }
{}

variable_reference::variable_reference(variable_reference const& other, object_creator creator) :
    variable_reference {
            clone_unique(other.name_, creator),
            other.region(),
    }
{}

variable_reference::variable_reference(variable_reference&& other, object_creator creator) :
    variable_reference {
            clone_unique(std::move(other.name_), creator),
            other.region(),
    }
{}

variable_reference* variable_reference::clone(object_creator creator) const& {
    return creator.create_object<variable_reference>(*this, creator);
}

variable_reference* variable_reference::clone(object_creator creator) && {
    return creator.create_object<variable_reference>(std::move(*this), creator);
}

expression::node_kind_type variable_reference::node_kind() const noexcept {
    return tag;
}

unique_object_ptr<name::name>& variable_reference::name() noexcept {
    return name_;
}

unique_object_ptr<name::name> const& variable_reference::name() const noexcept {
    return name_;
}

} // namespace mizugaki::ast::scalar
