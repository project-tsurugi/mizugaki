#include <mizugaki/ast/scalar/builtin_set_function_invocation.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::scalar {


using common::clone_vector;
using common::to_vector;

builtin_set_function_invocation::builtin_set_function_invocation(
        function_type function,
        std::optional<quantifier_type> quantifier,
        std::vector<operand_type> arguments,
        region_type region) noexcept :
    super { region },
    function_ { function },
    quantifier_ { quantifier },
    arguments_ { std::move(arguments) }
{}

builtin_set_function_invocation::builtin_set_function_invocation(
        function_type function,
        std::optional<quantifier_type> quantifier,
        common::rvalue_list<expression> arguments,
        region_type region) :
    builtin_set_function_invocation {
            function,
            quantifier,
            to_vector(arguments),
            region,
    }
{}

builtin_set_function_invocation::builtin_set_function_invocation(::takatori::util::clone_tag_t, builtin_set_function_invocation const& other) :
    builtin_set_function_invocation {
            other.function_,
            other.quantifier_,
            clone_vector(other.arguments_),
            other.region(),
    }
{}

builtin_set_function_invocation::builtin_set_function_invocation(::takatori::util::clone_tag_t, builtin_set_function_invocation&& other) :
    builtin_set_function_invocation {
            other.function_,
            other.quantifier_,
            clone_vector(std::move(other.arguments_)),
            other.region(),
    }
{}

builtin_set_function_invocation* builtin_set_function_invocation::clone() const& {
    return new builtin_set_function_invocation(::takatori::util::clone_tag, *this); // NOLINT
}

builtin_set_function_invocation* builtin_set_function_invocation::clone() && {
    return new builtin_set_function_invocation(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
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

std::vector<expression::operand_type>& builtin_set_function_invocation::arguments() noexcept {
    return arguments_;
}

std::vector<expression::operand_type> const& builtin_set_function_invocation::arguments() const noexcept {
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

void builtin_set_function_invocation::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "function"sv, function_);
    property(acceptor, "quantifier"sv, quantifier_);
    property(acceptor, "arguments"sv, arguments_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, builtin_set_function_invocation const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::scalar
