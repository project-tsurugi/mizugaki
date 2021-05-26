#include <mizugaki/ast/scalar/method_invocation.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::scalar {

using ::takatori::util::clone_unique;

using common::clone_vector;
using common::to_vector;

method_invocation::method_invocation(
        operand_type value,
        operator_kind_type operator_kind,
        std::unique_ptr<name::simple> name,
        std::vector<operand_type> arguments,
        region_type region) noexcept :
    super { region },
    value_ { std::move(value) },
    operator_kind_ { operator_kind },
    name_ { std::move(name) },
    arguments_ { std::move(arguments) }
{}

method_invocation::method_invocation(
        expression&& value,
        name::simple&& name,
        common::rvalue_list<expression> arguments,
        operator_kind_type operator_kind,
        region_type region) :
    method_invocation {
            clone_unique(std::move(value)),
            operator_kind,
            clone_unique(std::move(name)),
            to_vector(arguments),
            region,
    }
{}

method_invocation::method_invocation(::takatori::util::clone_tag_t, method_invocation const& other) :
    method_invocation {
            clone_unique(other.value_),
            other.operator_kind_,
            clone_unique(other.name_),
            clone_vector(other.arguments_),
            other.region(),
    }
{}

method_invocation::method_invocation(::takatori::util::clone_tag_t, method_invocation&& other) :
    method_invocation {
            clone_unique(std::move(other.value_)),
            other.operator_kind_,
            clone_unique(std::move(other.name_)),
            clone_vector(std::move(other.arguments_)),
            other.region(),
    }
{}

method_invocation* method_invocation::clone() const& {
    return new method_invocation(::takatori::util::clone_tag, *this); // NOLINT
}

method_invocation* method_invocation::clone() && {
    return new method_invocation(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
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

std::unique_ptr<name::simple>& method_invocation::name() noexcept {
    return name_;
}

std::unique_ptr<name::simple> const& method_invocation::name() const noexcept {
    return name_;
}

std::vector<expression::operand_type>& method_invocation::arguments() noexcept {
    return arguments_;
}

std::vector<expression::operand_type> const& method_invocation::arguments() const noexcept {
    return arguments_;
}

bool operator==(method_invocation const& a, method_invocation const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return false;
    }
    return eq(a.operator_kind_, b.operator_kind_)
            && eq(a.value_, b.value_)
            && eq(a.name_, b.name_)
            && eq(a.arguments_, b.arguments_);
}

bool operator!=(method_invocation const& a, method_invocation const& b) noexcept {
    return !(a == b);
}

bool method_invocation::equals(expression const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void method_invocation::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "value"sv, value_);
    property(acceptor, "operator_kind"sv, operator_kind_);
    property(acceptor, "name"sv, name_);
    property(acceptor, "arguments"sv, arguments_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, method_invocation const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::scalar
