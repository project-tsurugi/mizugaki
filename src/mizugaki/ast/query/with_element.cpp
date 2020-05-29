#include <mizugaki/ast/query/with_element.h>

namespace mizugaki::ast::query {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

using common::clone_vector;

with_element::with_element(
        unique_object_ptr<name::simple> name,
        unique_object_ptr<class expression> expression,
        common::vector<unique_object_ptr<name::simple>> column_names,
        region_type region) noexcept :
    element { region },
    name_ { std::move(name) },
    expression_ { std::move(expression) },
    column_names_ { std::move(column_names) }
{}

with_element::with_element(with_element const& other, object_creator creator) :
    with_element {
            clone_unique(*other.name_, creator),
            clone_unique(*other.expression_, creator),
            clone_vector(*other.column_names_, creator),
            other.region(),
    }
{}

with_element::with_element(with_element&& other, object_creator creator) :
    with_element {
            clone_unique(std::move(*other.name_), creator),
            clone_unique(std::move(*other.expression_), creator),
            clone_vector(std::move(*other.column_names_), creator),
            other.region(),
    }
{}

unique_object_ptr<name::simple>& with_element::name() noexcept {
    return *name_;
}

unique_object_ptr<name::simple> const& with_element::name() const noexcept {
    return *name_;
}

unique_object_ptr<class expression>& with_element::expression() noexcept {
    return *expression_;
}

unique_object_ptr<class expression> const& with_element::expression() const noexcept {
    return *expression_;
}

common::vector<unique_object_ptr<name::simple>>& with_element::column_names() noexcept {
    return *column_names_;
}

common::vector<unique_object_ptr<name::simple>> const& with_element::column_names() const noexcept {
    return *column_names_;
}

} // namespace mizugaki::ast::query
