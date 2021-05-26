#include <mizugaki/ast/type/user_defined.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::type {

using ::takatori::util::clone_unique;

user_defined::user_defined(
        std::unique_ptr<name::name> name,
        region_type region) noexcept:
    super { region },
    name_ { std::move(name) }
{}

user_defined::user_defined(
        name::name&& name,
        region_type region) :
    user_defined {
            clone_unique(std::move(name)),
            region,
    }
{}

user_defined::user_defined(::takatori::util::clone_tag_t, user_defined const& other) :
    user_defined {
            clone_unique(other.name_),
            other.region(),
    }
{}

user_defined::user_defined(::takatori::util::clone_tag_t, user_defined&& other) :
    user_defined {
            clone_unique(std::move(other.name_)),
            other.region(),
    }
{}

user_defined* user_defined::clone() const& {
    return new user_defined(::takatori::util::clone_tag, *this); // NOLINT
}

user_defined* user_defined::clone() && {
    return new user_defined(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

type::node_kind_type user_defined::node_kind() const noexcept {
    return tag;
}

std::unique_ptr<name::name>& user_defined::name() noexcept {
    return name_;
}

std::unique_ptr<name::name> const& user_defined::name() const noexcept {
    return name_;
}

bool operator==(user_defined const& a, user_defined const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return false;
    }
    return eq(a.name_, b.name_);
}

bool operator!=(user_defined const& a, user_defined const& b) noexcept {
    return !(a == b);
}

bool user_defined::equals(type const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void user_defined::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "name"sv, name_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, user_defined const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::type
