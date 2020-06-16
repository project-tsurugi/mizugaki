#include <mizugaki/ast/query/select_column.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::query {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::rvalue_ptr;
using ::takatori::util::unique_object_ptr;

select_column::select_column(
        unique_object_ptr<scalar::expression> value,
        unique_object_ptr<name::simple> name,
        region_type region) noexcept:
    super { region },
    value_ { std::move(value) },
    name_ { std::move(name) }
{}

select_column::select_column(
        scalar::expression&& value,
        rvalue_ptr<name::simple> name,
        region_type region) noexcept :
    select_column {
            clone_unique(std::move(value)),
            clone_unique(name),
            region,
    }
{}

select_column::select_column(select_column const& other, object_creator creator) :
    select_column {
            clone_unique(other.value_, creator),
            clone_unique(other.name_, creator),
            other.region(),
    }
{}

select_column::select_column(select_column&& other, object_creator creator) :
    select_column {
            clone_unique(std::move(other.value_), creator),
            clone_unique(std::move(other.name_), creator),
            other.region(),
    }
{}

select_column* select_column::clone(::takatori::util::object_creator creator) const& {
    return creator.create_object<select_column>(*this, creator);
}

select_column* select_column::clone(::takatori::util::object_creator creator)&& {
    return creator.create_object<select_column>(std::move(*this), creator);
}

select_element::node_kind_type select_column::node_kind() const noexcept {
    return tag;
}

unique_object_ptr<scalar::expression>& select_column::value() noexcept {
    return value_;
}

unique_object_ptr<scalar::expression> const& select_column::value() const noexcept {
    return value_;
}

unique_object_ptr<name::simple>& select_column::name() noexcept {
    return name_;
}

unique_object_ptr<name::simple> const& select_column::name() const noexcept {
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

} // namespace mizugaki::ast::query
