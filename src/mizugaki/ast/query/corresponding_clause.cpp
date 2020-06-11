#include <mizugaki/ast/query/corresponding_clause.h>

namespace mizugaki::ast::query {

using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

using common::clone_vector;

corresponding_clause::corresponding_clause(
        common::vector<unique_object_ptr<name::simple>> column_names,
        region_type region) noexcept :
    element { region },
    column_names_ { std::move(column_names) }
{}

corresponding_clause::corresponding_clause(corresponding_clause const& other, object_creator creator) :
    corresponding_clause {
            clone_vector(*other.column_names_, creator),
            other.region(),
    }
{}

corresponding_clause::corresponding_clause(corresponding_clause&& other, object_creator creator) :
    corresponding_clause {
            clone_vector(std::move(*other.column_names_), creator),
            other.region(),
    }
{}

common::vector<unique_object_ptr<name::simple>>& corresponding_clause::column_names() noexcept {
    return *column_names_;
}

common::vector<unique_object_ptr<name::simple>> const& corresponding_clause::column_names() const noexcept {
    return *column_names_;
}

} // namespace mizugaki::ast::query