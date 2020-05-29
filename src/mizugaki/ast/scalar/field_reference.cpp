#include <mizugaki/ast/scalar/field_reference.h>

#include <takatori/util/clonable.h>

namespace mizugaki::ast::scalar {

using operator_kind_type = field_reference::operator_kind_type;

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

field_reference::field_reference(
        operator_kind_type operator_kind,
        operand_type value,
        unique_object_ptr<name::simple> name,
        element::region_type region) noexcept:
    super { region },
    operator_kind_ { operator_kind },
    value_ { std::move(value) },
    name_ { std::move(name) }
{}

field_reference::field_reference(field_reference const& other, object_creator creator) :
    field_reference {
            other.operator_kind_,
            clone_unique(other.value_, creator),
            clone_unique(other.name_, creator),
            other.region(),
    }
{}

field_reference::field_reference(field_reference&& other, object_creator creator) :
    field_reference {
            other.operator_kind_,
            clone_unique(std::move(other.value_), creator),
            clone_unique(std::move(other.name_), creator),
            other.region(),
    }
{}

field_reference* field_reference::clone(object_creator creator) const& {
    return creator.create_object<field_reference>(*this, creator);
}

field_reference* field_reference::clone(object_creator creator) && {
    return creator.create_object<field_reference>(std::move(*this), creator);
}

expression::node_kind_type field_reference::node_kind() const noexcept {
    return tag;
}

operator_kind_type& field_reference::operator_kind() noexcept {
    return operator_kind_;
}

operator_kind_type const& field_reference::operator_kind() const noexcept {
    return operator_kind_;
}

expression::operand_type& field_reference::value() noexcept {
    return value_;
}

expression::operand_type const& field_reference::value() const noexcept {
    return value_;
}

unique_object_ptr<name::simple>& field_reference::name() noexcept {
    return name_;
}

unique_object_ptr<name::simple> const& field_reference::name() const noexcept {
    return name_;
}

} // namespace mizugaki::ast::scalar
