#include <mizugaki/ast/scalar/builtin_set_function_invocation.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::scalar {

using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

using common::clone_vector;

builtin_set_function_invocation::builtin_set_function_invocation(
        function_type function,
        std::optional<quantifier_type> quantifier,
        common::vector<operand_type> arguments,
        region_type region) noexcept:
    super { region },
    function_ { function },
    quantifier_ { std::move(quantifier) },
    arguments_ { std::move(arguments) }
{}

builtin_set_function_invocation::builtin_set_function_invocation(builtin_set_function_invocation const& other, object_creator creator) :
    builtin_set_function_invocation {
            other.function_,
            other.quantifier_,
            clone_vector(other.arguments_, creator),
            other.region(),
    }
{}

builtin_set_function_invocation::builtin_set_function_invocation(builtin_set_function_invocation&& other, object_creator creator) :
    builtin_set_function_invocation {
            other.function_,
            other.quantifier_,
            clone_vector(std::move(other.arguments_), creator),
            other.region(),
    }
{}

builtin_set_function_invocation* builtin_set_function_invocation::clone(object_creator creator) const& {
    return creator.create_object<builtin_set_function_invocation>(*this, creator);
}

builtin_set_function_invocation* builtin_set_function_invocation::clone(object_creator creator) && {
    return creator.create_object<builtin_set_function_invocation>(std::move(*this), creator);
}

expression::node_kind_type builtin_set_function_invocation::node_kind() const noexcept {
    return tag;
}

builtin_set_function_invocation::function_type& builtin_set_function_invocation::function() noexcept {
    return function_;
}

builtin_set_function_invocation::function_type const& builtin_set_function_invocation::function() const noexcept {
    return function_;
}

constexpr std::optional<builtin_set_function_invocation::quantifier_type>& builtin_set_function_invocation::quantifier() noexcept {
    return quantifier_;
}

constexpr std::optional<builtin_set_function_invocation::quantifier_type> const& builtin_set_function_invocation::quantifier() const noexcept {
    return quantifier_;
}

common::vector<expression::operand_type>& builtin_set_function_invocation::arguments() noexcept {
    return arguments_;
}

common::vector<expression::operand_type> const& builtin_set_function_invocation::arguments() const noexcept {
    return arguments_;
}

bool operator==(builtin_set_function_invocation const& a, builtin_set_function_invocation const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return false;
    }
    return eq(a.function_, b.function_)
            && eq(a.quantifier_, b.quantifier_)
            && eq(a.arguments_, b.arguments_);
}

bool operator!=(builtin_set_function_invocation const& a, builtin_set_function_invocation const& b) noexcept {
    return !(a == b);
}

bool builtin_set_function_invocation::equals(expression const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

} // namespace mizugaki::ast::scalar
