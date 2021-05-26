#include <mizugaki/ast/scalar/host_parameter_reference.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::scalar {

using ::takatori::util::clone_unique;

host_parameter_reference::host_parameter_reference(
        std::unique_ptr<name::simple> name,
        region_type region) noexcept:
    super { region },
    name_ { std::move(name) }
{}

host_parameter_reference::host_parameter_reference(
        name::simple&& name,
        region_type region) :
    host_parameter_reference {
            clone_unique(std::move(name)),
            region,
    }
{}

host_parameter_reference::host_parameter_reference(::takatori::util::clone_tag_t, host_parameter_reference const& other) :
    host_parameter_reference {
            clone_unique(other.name_),
            other.region(),
    }
{}

host_parameter_reference::host_parameter_reference(::takatori::util::clone_tag_t, host_parameter_reference&& other) :
    host_parameter_reference {
            clone_unique(std::move(other.name_)),
            other.region(),
    }
{}

host_parameter_reference* host_parameter_reference::clone() const& {
    return new host_parameter_reference(::takatori::util::clone_tag, *this); // NOLINT
}

host_parameter_reference* host_parameter_reference::clone() && {
    return new host_parameter_reference(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

expression::node_kind_type host_parameter_reference::node_kind() const noexcept {
    return tag;
}

std::unique_ptr<name::simple>& host_parameter_reference::name() noexcept {
    return name_;
}

std::unique_ptr<name::simple> const& host_parameter_reference::name() const noexcept {
    return name_;
}

bool operator==(host_parameter_reference const& a, host_parameter_reference const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return false;
    }
    return eq(a.name_, b.name_);
}

bool operator!=(host_parameter_reference const& a, host_parameter_reference const& b) noexcept {
    return !(a == b);
}

bool host_parameter_reference::equals(expression const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void host_parameter_reference::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "name"sv, name_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, host_parameter_reference const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::scalar
