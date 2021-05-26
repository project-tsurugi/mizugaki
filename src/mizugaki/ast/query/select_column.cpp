#include <mizugaki/ast/query/select_column.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::query {

using ::takatori::util::clone_unique;
using ::takatori::util::rvalue_ptr;

select_column::select_column(
        std::unique_ptr<scalar::expression> value,
        std::unique_ptr<name::simple> name,
        region_type region) noexcept:
    super { region },
    value_ { std::move(value) },
    name_ { std::move(name) }
{}

select_column::select_column(
        scalar::expression&& value,
        rvalue_ptr<name::simple> name,
        region_type region) :
    select_column {
            clone_unique(std::move(value)),
            clone_unique(name),
            region,
    }
{}

select_column::select_column(::takatori::util::clone_tag_t, select_column const& other) :
    select_column {
            clone_unique(other.value_),
            clone_unique(other.name_),
            other.region(),
    }
{}

select_column::select_column(::takatori::util::clone_tag_t, select_column&& other) :
    select_column {
            clone_unique(std::move(other.value_)),
            clone_unique(std::move(other.name_)),
            other.region(),
    }
{}

select_column* select_column::clone() const& {
    return new select_column(::takatori::util::clone_tag, *this); // NOLINT
}

select_column* select_column::clone()&& {
    return new select_column(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

select_element::node_kind_type select_column::node_kind() const noexcept {
    return tag;
}

std::unique_ptr<scalar::expression>& select_column::value() noexcept {
    return value_;
}

std::unique_ptr<scalar::expression> const& select_column::value() const noexcept {
    return value_;
}

std::unique_ptr<name::simple>& select_column::name() noexcept {
    return name_;
}

std::unique_ptr<name::simple> const& select_column::name() const noexcept {
    return name_;
}

bool operator==(select_column const& a, select_column const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return false;
    }
    return eq(a.value_, b.value_)
            && eq(a.name_, b.name_);
}

bool operator!=(select_column const& a, select_column const& b) noexcept {
    return !(a == b);
}

bool select_column::equals(select_element const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void select_column::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "value"sv, value_);
    property(acceptor, "name"sv, name_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, select_column const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::query
