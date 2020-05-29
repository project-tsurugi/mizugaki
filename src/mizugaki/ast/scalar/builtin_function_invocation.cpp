#include <mizugaki/ast/scalar/builtin_function_invocation.h>

namespace mizugaki::ast::scalar {

using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

using common::clone_vector;

builtin_function_invocation::builtin_function_invocation(
        function_type function,
        common::vector<operand_type> arguments,
        region_type region) noexcept:
    super { region },
    function_ { function },
    arguments_ { std::move(arguments) }
{}


builtin_function_invocation::builtin_function_invocation(builtin_function_invocation const& other, object_creator creator) :
    builtin_function_invocation {
            other.function_,
            clone_vector(other.arguments_, creator),
            other.region(),
    }
{}

builtin_function_invocation::builtin_function_invocation(builtin_function_invocation&& other, object_creator creator) :
    builtin_function_invocation {
            other.function_,
            clone_vector(std::move(other.arguments_), creator),
            other.region(),
    }
{}

builtin_function_invocation* builtin_function_invocation::clone(object_creator creator) const& {
    return creator.create_object<builtin_function_invocation>(*this, creator);
}

builtin_function_invocation* builtin_function_invocation::clone(object_creator creator) && {
    return creator.create_object<builtin_function_invocation>(std::move(*this), creator);
}

expression::node_kind_type builtin_function_invocation::node_kind() const noexcept {
    return tag;
}

builtin_function_invocation::function_type& builtin_function_invocation::function() noexcept {
    return function_;
}

builtin_function_invocation::function_type const& builtin_function_invocation::function() const noexcept {
    return function_;
}

common::vector<expression::operand_type>& builtin_function_invocation::arguments() noexcept {
    return arguments_;
}

common::vector<expression::operand_type> const& builtin_function_invocation::arguments() const noexcept {
    return arguments_;
}

} // namespace mizugaki::ast::scalar
