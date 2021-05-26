#include <mizugaki/ast/table/table_reference.h>

#include <mizugaki/ast/common/optional.h>
#include <mizugaki/ast/common/serializers.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::table {

using ::takatori::util::clone_unique;

using common::clone_optional;

table_reference::table_reference(
        bool_type is_only,
        std::unique_ptr<name::name> name,
        std::optional<correlation_type> correlation,
        region_type region) noexcept:
    super { region },
    is_only_ { is_only },
    name_ { std::move(name) },
    correlation_ { std::move(correlation) }
{}

table_reference::table_reference(
        name::name&& name,
        std::optional<correlation_type> correlation,
        bool_type is_only,
        region_type region) :
    table_reference {
            is_only,
            clone_unique(std::move(name)),
            std::move(correlation),
            region,
    }
{}

table_reference::table_reference(::takatori::util::clone_tag_t, table_reference const& other) :
    table_reference {
            other.is_only_,
            clone_unique(other.name_),
            clone_optional(other.correlation_),
            other.region(),
    }
{}

table_reference::table_reference(::takatori::util::clone_tag_t, table_reference&& other) :
    table_reference {
            other.is_only_,
            clone_unique(std::move(other.name_)),
            clone_optional(std::move(other.correlation_)),
            other.region(),
    }
{}

table_reference* table_reference::clone() const& {
    return new table_reference(::takatori::util::clone_tag, *this); // NOLINT
}

table_reference* table_reference::clone() && {
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

std::optional<table_reference::correlation_type>& table_reference::correlation() noexcept {
    return correlation_;
}

std::optional<table_reference::correlation_type> const& table_reference::correlation() const noexcept {
    return correlation_;
}

table_reference::bool_type& table_reference::is_only() noexcept {
    return is_only_;
}

table_reference::bool_type const& table_reference::is_only() const noexcept {
    return is_only_;
}

bool operator==(table_reference const& a, table_reference const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return true;
    }
    return eq(a.name_, b.name_)
            && eq(a.correlation_, b.correlation_)
            && eq(a.is_only_, a.is_only_);
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
    property(acceptor, "is_only"sv, is_only_);
    property(acceptor, "name"sv, name_);
    property(acceptor, "correlation"sv, correlation_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, table_reference const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::table
