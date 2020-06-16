#include <mizugaki/ast/scalar/function_invocation.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::scalar {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

using common::clone_vector;

function_invocation::function_invocation(
        unique_object_ptr<name::name> name,
        common::vector<operand_type> arguments,
        region_type region) noexcept:
    super { region },
    name_ { std::move(name) },
    arguments_ { std::move(arguments) }
{}

function_invocation::function_invocation(function_invocation const& other, object_creator creator) :
    function_invocation {
            clone_unique(other.name_, creator),
            clone_vector(other.arguments_, creator),
            other.region(),
    }
{}

function_invocation::function_invocation(function_invocation&& other, object_creator creator) :
    function_invocation {
            clone_unique(std::move(other.name_), creator),
            clone_vector(std::move(other.arguments_), creator),
            other.region(),
    }
{}

function_invocation* function_invocation::clone(object_creator creator) const& {
    return creator.create_object<function_invocation>(*this, creator);
}

function_invocation* function_invocation::clone(object_creator creator) && {
    return creator.create_object<function_invocation>(std::move(*this), creator);
}

expression::node_kind_type function_invocation::node_kind() const noexcept {
    return tag;
}

::takatori::util::unique_object_ptr<name::name>& function_invocation::name() noexcept {
    return name_;
}

::takatori::util::unique_object_ptr<name::name> const& function_invocation::name() const noexcept {
    return name_;
}

common::vector<expression::operand_type>& function_invocation::arguments() noexcept {
    return arguments_;
}

common::vector<expression::operand_type> const& function_invocation::arguments() const noexcept {
    return arguments_;
}

bool operator==(function_invocation const& a, function_invocation const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return false;
    }
    return eq(a.name_, b.name_)
            && eq(a.arguments_, b.arguments_);
}

bool operator!=(function_invocation const& a, function_invocation const& b) noexcept {
    return !(a == b);
}

bool function_invocation::equals(expression const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

} // namespace mizugaki::ast::scalar
