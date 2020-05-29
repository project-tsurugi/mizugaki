#include <mizugaki/ast/table/correlation_clause.h>

#include <takatori/util/clonable.h>

namespace mizugaki::ast::table {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

using common::clone_vector;

correlation_clause::correlation_clause(
        unique_object_ptr<name::simple> correlation_name,
        common::vector<unique_object_ptr<name::simple>> column_names,
        element::region_type region) noexcept:
    element { region },
    correlation_name_ { std::move(correlation_name) },
    column_names_ { std::move(column_names) }
{}

correlation_clause::correlation_clause(correlation_clause const& other, object_creator creator) :
    correlation_clause {
            clone_unique(*other.correlation_name_, creator),
            clone_vector(*other.column_names_, creator),
            other.region(),
    }
{}

correlation_clause::correlation_clause(correlation_clause&& other, object_creator creator) :
    correlation_clause {
            clone_unique(std::move(*other.correlation_name_), creator),
            clone_vector(std::move(*other.column_names_), creator),
            other.region(),
    }
{}

unique_object_ptr<name::simple>& correlation_clause::correlation_name() noexcept {
    return *correlation_name_;
}

unique_object_ptr<name::simple> const& correlation_clause::correlation_name() const noexcept {
    return *correlation_name_;
}

common::vector<unique_object_ptr<name::simple>>& correlation_clause::column_names() noexcept {
    return *column_names_;
}

common::vector<unique_object_ptr<name::simple>> const& correlation_clause::column_names() const noexcept {
    return *column_names_;
}

} // namespace mizugaki::ast::table
