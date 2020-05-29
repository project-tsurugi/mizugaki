#include <mizugaki/ast/query/grouping_column.h>

#include <takatori/util/clonable.h>

namespace mizugaki::ast::query {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

grouping_column::grouping_column(
        unique_object_ptr<scalar::expression> column,
        unique_object_ptr<name::name> collation,
        region_type region) noexcept :
    super { region },
    column_ { std::move(column) },
    collation_ { std::move(collation) }
{}

grouping_column::grouping_column(grouping_column const& other, object_creator creator) :
    grouping_column {
            clone_unique(other.column_, creator),
            clone_unique(other.collation_, creator),
            other.region(),
    }
{}

grouping_column::grouping_column(grouping_column&& other, object_creator creator) :
    grouping_column {
            clone_unique(std::move(other.column_), creator),
            clone_unique(std::move(other.collation_), creator),
            other.region(),
    }
{}

grouping_column* grouping_column::clone(object_creator creator) const& {
    return creator.create_object<grouping_column>(*this, creator);
}

grouping_column* grouping_column::clone(object_creator creator)&& {
    return creator.create_object<grouping_column>(std::move(*this), creator);
}

grouping_element::node_kind_type grouping_column::node_kind() const noexcept {
    return tag;
}

unique_object_ptr<scalar::expression>& grouping_column::column() noexcept {
    return column_;
}

unique_object_ptr<scalar::expression> const& grouping_column::column() const noexcept {
    return column_;
}

unique_object_ptr<name::name>& grouping_column::collation() noexcept {
    return collation_;
}

unique_object_ptr<name::name> const& grouping_column::collation() const noexcept {
    return collation_;
}

} // namespace mizugaki::ast::query
