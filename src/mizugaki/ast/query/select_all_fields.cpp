#include <mizugaki/ast/query/select_all_fields.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::query {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

select_all_fields::select_all_fields(
        unique_object_ptr<scalar::expression> value,
        region_type region) noexcept :
    super { region },
    value_ { std::move(value) }
{}

select_all_fields::select_all_fields(select_all_fields const& other, object_creator creator) :
    select_all_fields {
            clone_unique(other.value_, creator),
            other.region(),
    }
{}

select_all_fields::select_all_fields(select_all_fields&& other, object_creator creator) :
    select_all_fields {
            clone_unique(std::move(other.value_), creator),
            other.region(),
    }
{}

select_all_fields* select_all_fields::clone(::takatori::util::object_creator creator) const& {
    return creator.create_object<select_all_fields>(*this, creator);
}

select_all_fields* select_all_fields::clone(::takatori::util::object_creator creator)&& {
    return creator.create_object<select_all_fields>(std::move(*this), creator);
}

select_element::node_kind_type select_all_fields::node_kind() const noexcept {
    return tag;
}

unique_object_ptr<scalar::expression>& select_all_fields::value() noexcept {
    return value_;
}

unique_object_ptr<scalar::expression> const& select_all_fields::value() const noexcept {
    return value_;
}

bool operator==(select_all_fields const& a, select_all_fields const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return false;
    }
    return eq(a.value_, b.value_);
}

bool operator!=(select_all_fields const& a, select_all_fields const& b) noexcept {
    return !(a == b);
}

bool select_all_fields::equals(select_element const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

} // namespace mizugaki::ast::query
