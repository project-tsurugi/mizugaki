#include <mizugaki/ast/scalar/set_function_invocation.h>

#include <takatori/util/clonable.h>

namespace mizugaki::ast::scalar {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

using common::clone_vector;

set_function_invocation::set_function_invocation(
        unique_object_ptr<name::name> name,
        std::optional<quantifier_type> quantifier,
        common::vector<operand_type> arguments,
        element::region_type region) noexcept:
    super { region },
    name_ { std::move(name) },
    quantifier_ { std::move(quantifier) },
    arguments_ { std::move(arguments) }
{}

set_function_invocation::set_function_invocation(set_function_invocation const& other, object_creator creator) :
    set_function_invocation {
            clone_unique(other.name_, creator),
            other.quantifier_,
            clone_vector(other.arguments_, creator),
            other.region(),
    }
{}

set_function_invocation::set_function_invocation(set_function_invocation&& other, object_creator creator) :
    set_function_invocation {
            clone_unique(std::move(other.name_), creator),
            other.quantifier_,
            clone_vector(std::move(other.arguments_), creator),
            other.region(),
    }
{}

set_function_invocation* set_function_invocation::clone(object_creator creator) const& {
    return creator.create_object<set_function_invocation>(*this, creator);
}

set_function_invocation* set_function_invocation::clone(object_creator creator) && {
    return creator.create_object<set_function_invocation>(std::move(*this), creator);
}

expression::node_kind_type set_function_invocation::node_kind() const noexcept {
    return tag;
}

::takatori::util::unique_object_ptr<name::name>& set_function_invocation::name() noexcept {
    return name_;
}

::takatori::util::unique_object_ptr<name::name> const& set_function_invocation::name() const noexcept {
    return name_;
}

constexpr std::optional<set_function_invocation::quantifier_type>& set_function_invocation::quantifier() noexcept {
    return quantifier_;
}

constexpr std::optional<set_function_invocation::quantifier_type> const& set_function_invocation::quantifier() const noexcept {
    return quantifier_;
}

common::vector<expression::operand_type>& set_function_invocation::arguments() noexcept {
    return arguments_;
}

common::vector<expression::operand_type> const& set_function_invocation::arguments() const noexcept {
    return arguments_;
}

} // namespace mizugaki::ast::scalar
