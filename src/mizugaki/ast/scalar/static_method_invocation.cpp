#include <mizugaki/ast/scalar/static_method_invocation.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::scalar {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

using common::clone_vector;
using common::to_vector;

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

static_method_invocation::static_method_invocation(
        type::type&& type,
        name::simple&& name,
        common::rvalue_list<expression> arguments,
        region_type region) :
    static_method_invocation {
            clone_unique(std::move(type)),
            clone_unique(std::move(name)),
            to_vector(arguments),
            region,
    }
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

bool operator==(static_method_invocation const& a, static_method_invocation const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return false;
    }
    return eq(a.type_, b.type_)
            && eq(a.name_, b.name_)
            && eq(a.arguments_, b.arguments_);
}

bool operator!=(static_method_invocation const& a, static_method_invocation const& b) noexcept {
    return !(a == b);
}

bool static_method_invocation::equals(expression const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void static_method_invocation::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "type"sv, type_);
    property(acceptor, "name"sv, name_);
    property(acceptor, "arguments"sv, arguments_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, static_method_invocation const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::scalar
