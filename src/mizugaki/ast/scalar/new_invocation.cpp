#include <mizugaki/ast/scalar/new_invocation.h>

#include <takatori/util/clonable.h>

namespace mizugaki::ast::scalar {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

using common::clone_vector;

new_invocation::new_invocation(
        unique_object_ptr<type::type> type,
        common::vector<operand_type> arguments,
        region_type region) noexcept:
    super { region },
    type_ { std::move(type) },
    arguments_ { std::move(arguments) }
{}

new_invocation::new_invocation(new_invocation const& other, object_creator creator) :
    new_invocation {
            clone_unique(other.type_, creator),
            clone_vector(other.arguments_, creator),
            other.region(),
    }
{}

new_invocation::new_invocation(new_invocation&& other, object_creator creator) :
    new_invocation {
            clone_unique(std::move(other.type_), creator),
            clone_vector(std::move(other.arguments_), creator),
            other.region(),
    }
{}

new_invocation* new_invocation::clone(object_creator creator) const& {
    return creator.create_object<new_invocation>(*this, creator);
}

new_invocation* new_invocation::clone(object_creator creator) && {
    return creator.create_object<new_invocation>(std::move(*this), creator);
}

expression::node_kind_type new_invocation::node_kind() const noexcept {
    return tag;
}

::takatori::util::unique_object_ptr<type::type>& new_invocation::type() noexcept {
    return type_;
}

::takatori::util::unique_object_ptr<type::type> const& new_invocation::type() const noexcept {
    return type_;
}

common::vector<expression::operand_type>& new_invocation::arguments() noexcept {
    return arguments_;
}

common::vector<expression::operand_type> const& new_invocation::arguments() const noexcept {
    return arguments_;
}

} // namespace mizugaki::ast::scalar
