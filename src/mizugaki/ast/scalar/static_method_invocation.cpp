#include <mizugaki/ast/scalar/static_method_invocation.h>

#include <takatori/util/clonable.h>

namespace mizugaki::ast::scalar {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

using common::clone_vector;

static_method_invocation::static_method_invocation(
        unique_object_ptr<type::type> type,
        unique_object_ptr<name::simple> name,
        common::vector<operand_type> arguments,
        region_type region) noexcept:
    super { region },
    type_ { std::move(type) },
    name_ { std::move(name) },
    arguments_ { std::move(arguments) }
{}

static_method_invocation::static_method_invocation(static_method_invocation const& other, object_creator creator) :
    static_method_invocation {
            clone_unique(other.type_, creator),
            clone_unique(other.name_, creator),
            clone_vector(other.arguments_, creator),
            other.region(),
    }
{}

static_method_invocation::static_method_invocation(static_method_invocation&& other, object_creator creator) :
    static_method_invocation {
            clone_unique(std::move(other.type_), creator),
            clone_unique(std::move(other.name_), creator),
            clone_vector(std::move(other.arguments_), creator),
            other.region(),
    }
{}

static_method_invocation* static_method_invocation::clone(object_creator creator) const& {
    return creator.create_object<static_method_invocation>(*this, creator);
}

static_method_invocation* static_method_invocation::clone(object_creator creator) && {
    return creator.create_object<static_method_invocation>(std::move(*this), creator);
}

expression::node_kind_type static_method_invocation::node_kind() const noexcept {
    return tag;
}

::takatori::util::unique_object_ptr<type::type>& static_method_invocation::type() noexcept {
    return type_;
}

::takatori::util::unique_object_ptr<type::type> const& static_method_invocation::type() const noexcept {
    return type_;
}

::takatori::util::unique_object_ptr<name::simple>& static_method_invocation::name() noexcept {
    return name_;
}

::takatori::util::unique_object_ptr<name::simple> const& static_method_invocation::name() const noexcept {
    return name_;
}

common::vector<expression::operand_type>& static_method_invocation::arguments() noexcept {
    return arguments_;
}

common::vector<expression::operand_type> const& static_method_invocation::arguments() const noexcept {
    return arguments_;
}

} // namespace mizugaki::ast::scalar
