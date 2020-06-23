#include <mizugaki/ast/scalar/new_invocation.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::scalar {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

using common::clone_vector;
using common::to_vector;

new_invocation::new_invocation(
        unique_object_ptr<type::type> type,
        common::vector<operand_type> arguments,
        region_type region) noexcept :
    super { region },
    type_ { std::move(type) },
    arguments_ { std::move(arguments) }
{}

new_invocation::new_invocation(
        type::type&& type,
        common::rvalue_list<expression> arguments,
        region_type region) :
    new_invocation {
            clone_unique(std::move(type)),
            to_vector(arguments),
            region,
    }
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

bool operator==(new_invocation const& a, new_invocation const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return false;
    }
    return eq(a.type_, b.type_)
            && eq(a.arguments_, b.arguments_);
}

bool operator!=(new_invocation const& a, new_invocation const& b) noexcept {
    return !(a == b);
}

bool new_invocation::equals(expression const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void new_invocation::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "type"sv, type_);
    property(acceptor, "arguments"sv, arguments_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, new_invocation const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::scalar
