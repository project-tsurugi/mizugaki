#include <mizugaki/ast/scalar/static_method_invocation.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::scalar {

using ::takatori::util::clone_unique;

using common::clone_vector;
using common::to_vector;

static_method_invocation::static_method_invocation(
        std::unique_ptr<type::type> type,
        std::unique_ptr<name::simple> name,
        std::vector<operand_type> arguments,
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

static_method_invocation::static_method_invocation(::takatori::util::clone_tag_t, static_method_invocation const& other) :
    static_method_invocation {
            clone_unique(other.type_),
            clone_unique(other.name_),
            clone_vector(other.arguments_),
            other.region(),
    }
{}

static_method_invocation::static_method_invocation(::takatori::util::clone_tag_t, static_method_invocation&& other) :
    static_method_invocation {
            clone_unique(std::move(other.type_)),
            clone_unique(std::move(other.name_)),
            clone_vector(std::move(other.arguments_)),
            other.region(),
    }
{}

static_method_invocation* static_method_invocation::clone() const& {
    return new static_method_invocation(::takatori::util::clone_tag, *this); // NOLINT
}

static_method_invocation* static_method_invocation::clone() && {
    return new static_method_invocation(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

expression::node_kind_type static_method_invocation::node_kind() const noexcept {
    return tag;
}

std::unique_ptr<type::type>& static_method_invocation::type() noexcept {
    return type_;
}

std::unique_ptr<type::type> const& static_method_invocation::type() const noexcept {
    return type_;
}

std::unique_ptr<name::simple>& static_method_invocation::name() noexcept {
    return name_;
}

std::unique_ptr<name::simple> const& static_method_invocation::name() const noexcept {
    return name_;
}

std::vector<expression::operand_type>& static_method_invocation::arguments() noexcept {
    return arguments_;
}

std::vector<expression::operand_type> const& static_method_invocation::arguments() const noexcept {
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
