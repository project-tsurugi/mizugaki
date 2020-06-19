#include <mizugaki/ast/query/group_by_clause.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::query {

using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

using common::clone_vector;
using common::to_vector;

group_by_clause::group_by_clause(
        common::vector<unique_object_ptr<element_type>> elements,
        region_type region) noexcept :
    element { region },
    elements_ { std::move(elements) }
{}

group_by_clause::group_by_clause(
        common::rvalue_list<element_type> elements,
        region_type region) noexcept :
    group_by_clause {
            to_vector(elements),
            region,
    }
{}

group_by_clause::group_by_clause(group_by_clause const& other, object_creator creator) :
    group_by_clause {
            clone_vector(*other.elements_, creator),
            other.region(),
    }
{}

group_by_clause::group_by_clause(group_by_clause&& other, object_creator creator) :
    group_by_clause {
            clone_vector(std::move(*other.elements_), creator),
            other.region(),
    }
{}

common::vector<unique_object_ptr<group_by_clause::element_type>>& group_by_clause::elements() noexcept {
    return *elements_;
}

common::vector<unique_object_ptr<group_by_clause::element_type>> const& group_by_clause::elements() const noexcept {
    return *elements_;
}

bool operator==(group_by_clause const& a, group_by_clause const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return false;
    }
    return eq(*a.elements_, *b.elements_);
}

bool operator!=(group_by_clause const& a, group_by_clause const& b) noexcept {
    return !(a == b);
}

} // namespace mizugaki::ast::query
