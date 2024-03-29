#include <mizugaki/ast/query/table_reference.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::query {

using ::takatori::util::clone_unique;

table_reference::table_reference(
        std::unique_ptr<name::name> name,
        region_type region) noexcept :
    super { region },
    name_ { std::move(name) }
{}


table_reference::table_reference(
        name::name&& name,
        region_type region) noexcept :
    table_reference {
            clone_unique(std::move(name)),
            region,
    }
{}

table_reference::table_reference(::takatori::util::clone_tag_t, table_reference const& other) :
    table_reference {
            clone_unique(other.name_),
            other.region(),
    }
{}

table_reference::table_reference(::takatori::util::clone_tag_t, table_reference&& other) :
    table_reference {
            clone_unique(std::move(other.name_)),
            other.region(),
    }
{}

table_reference* table_reference::clone() const& {
    return new table_reference(::takatori::util::clone_tag, *this); // NOLINT
}

table_reference* table_reference::clone()&& {
    return new table_reference(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

expression::node_kind_type table_reference::node_kind() const noexcept {
    return tag;
}

std::unique_ptr<name::name>& table_reference::name() noexcept {
    return name_;
}

std::unique_ptr<name::name> const& table_reference::name() const noexcept {
    return name_;
}

bool operator==(table_reference const& a, table_reference const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return false;
    }
    return eq(a.name_, b.name_);
}

bool operator!=(table_reference const& a, table_reference const& b) noexcept {
    return !(a == b);
}

bool table_reference::equals(expression const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void table_reference::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "name"sv, name_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, table_reference const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::query
