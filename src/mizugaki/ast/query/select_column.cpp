#include <mizugaki/ast/query/select_column.h>

#include <takatori/util/clonable.h>

namespace mizugaki::ast::query {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

select_column::select_column(
        unique_object_ptr<scalar::expression> value,
        unique_object_ptr<name::simple> name,
        region_type region) noexcept:
    super { region },
    value_ { std::move(value) },
    name_ { std::move(name) }
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

} // namespace mizugaki::ast::query