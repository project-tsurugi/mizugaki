#include <mizugaki/ast/scalar/method_invocation.h>

#include <takatori/util/clonable.h>

namespace mizugaki::ast::scalar {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

using common::clone_vector;

method_invocation::method_invocation(
        operator_kind_type operator_kind,
        operand_type value,
        unique_object_ptr<name::simple> name,
        common::vector<operand_type> arguments,
        region_type region) noexcept:
    super { region },
    operator_kind_ { operator_kind },
    value_ { std::move(value) },
    name_ { std::move(name) },
    arguments_ { std::move(arguments) }
{}

method_invocation::method_invocation(method_invocation const& other, object_creator creator) :
    method_invocation {
            other.operator_kind_,
            clone_unique(other.value_, creator),
            clone_unique(other.name_, creator),
            clone_vector(other.arguments_, creator),
            other.region(),
    }
{}

method_invocation::method_invocation(method_invocation&& other, object_creator creator) :
    method_invocation {
            other.operator_kind_,
            clone_unique(std::move(other.value_), creator),
            clone_unique(std::move(other.name_), creator),
            clone_vector(std::move(other.arguments_), creator),
            other.region(),
    }
{}

method_invocation* method_invocation::clone(object_creator creator) const& {
    return creator.create_object<method_invocation>(*this, creator);
}

method_invocation* method_invocation::clone(object_creator creator) && {
    return creator.create_object<method_invocation>(std::move(*this), creator);
}

expression::node_kind_type method_invocation::node_kind() const noexcept {
    return tag;
}

method_invocation::operator_kind_type& method_invocation::operator_kind() noexcept {
    return operator_kind_;
}

method_invocation::operator_kind_type const& method_invocation::operator_kind() const noexcept {
    return operator_kind_;
}

expression::operand_type& method_invocation::value() noexcept {
    return value_;
}

expression::operand_type const& method_invocation::value() const noexcept {
    return value_;
}

::takatori::util::unique_object_ptr<name::simple>& method_invocation::name() noexcept {
    return name_;
}

::takatori::util::unique_object_ptr<name::simple> const& method_invocation::name() const noexcept {
    return name_;
}

common::vector<expression::operand_type>& method_invocation::arguments() noexcept {
    return arguments_;
}

common::vector<expression::operand_type> const& method_invocation::arguments() const noexcept {
    return arguments_;
}

} // namespace mizugaki::ast::scalar
