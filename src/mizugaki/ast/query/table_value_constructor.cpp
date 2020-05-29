#include <mizugaki/ast/query/table_value_constructor.h>

namespace mizugaki::ast::query {

using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

using common::clone_vector;

table_value_constructor::table_value_constructor(
        common::vector<unique_object_ptr<scalar::expression>> elements,
        region_type region) noexcept :
    super { region },
    elements_ { std::move(elements) }
{}

table_value_constructor::table_value_constructor(table_value_constructor const& other, object_creator creator) :
    table_value_constructor {
            clone_vector(other.elements_, creator),
            other.region(),
    }
{}

table_value_constructor::table_value_constructor(table_value_constructor&& other, object_creator creator) :
    table_value_constructor {
            clone_vector(std::move(other.elements_), creator),
            other.region(),
    }
{}

table_value_constructor* table_value_constructor::clone(object_creator creator) const& {
    return creator.create_object<table_value_constructor>(*this, creator);
}

table_value_constructor* table_value_constructor::clone(object_creator creator)&& {
    return creator.create_object<table_value_constructor>(std::move(*this), creator);
}

expression::node_kind_type table_value_constructor::node_kind() const noexcept {
    return tag;
}

common::vector<unique_object_ptr<scalar::expression>>& table_value_constructor::elements() noexcept {
    return elements_;
}

common::vector<unique_object_ptr<scalar::expression>> const& table_value_constructor::elements() const noexcept {
    return elements_;
}

} // namespace mizugaki::ast::query
