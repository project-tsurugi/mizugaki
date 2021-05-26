#include <mizugaki/ast/scalar/variable_reference.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::scalar {

using ::takatori::util::clone_unique;

variable_reference::variable_reference(
        std::unique_ptr<name::name> name,
        region_type region) noexcept:
    super { region },
    name_ { std::move(name) }
{}

variable_reference::variable_reference(
        name::name&& name,
        region_type region) :
    variable_reference {
            clone_unique(std::move(name)),
            region,
    }
{}

variable_reference::variable_reference(::takatori::util::clone_tag_t, variable_reference const& other) :
    variable_reference {
            clone_unique(other.name_),
            other.region(),
    }
{}

variable_reference::variable_reference(::takatori::util::clone_tag_t, variable_reference&& other) :
    variable_reference {
            clone_unique(std::move(other.name_)),
            other.region(),
    }
{}

variable_reference* variable_reference::clone() const& {
    return new variable_reference(::takatori::util::clone_tag, *this); // NOLINT
}

variable_reference* variable_reference::clone() && {
    return new variable_reference(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

expression::node_kind_type variable_reference::node_kind() const noexcept {
    return tag;
}

std::unique_ptr<name::name>& variable_reference::name() noexcept {
    return name_;
}

std::unique_ptr<name::name> const& variable_reference::name() const noexcept {
    return name_;
}

bool operator==(variable_reference const& a, variable_reference const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return false;
    }
    return eq(a.name_, b.name_);
}

bool operator!=(variable_reference const& a, variable_reference const& b) noexcept {
    return !(a == b);
}

bool variable_reference::equals(expression const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void variable_reference::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "name"sv, name_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, variable_reference const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::scalar
