#include <mizugaki/ast/table/correlation_clause.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::table {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

using common::clone_vector;
using common::to_vector;

correlation_clause::correlation_clause(
        unique_object_ptr<name::simple> correlation_name,
        common::vector<unique_object_ptr<name::simple>> column_names,
        element::region_type region) noexcept:
    element { region },
    correlation_name_ { std::move(correlation_name) },
    column_names_ { std::move(column_names) }
{}

correlation_clause::correlation_clause(
        name::simple&& correlation_name,
        common::rvalue_list<name::simple> column_names,
        region_type region) :
    correlation_clause {
            clone_unique(std::move(correlation_name)),
            to_vector(column_names),
            region,
    }
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

bool operator==(correlation_clause const& a, correlation_clause const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return true;
    }
    return eq(*a.correlation_name_, *b.correlation_name_)
            && eq(*a.column_names_, *b.column_names_);
}

bool operator!=(correlation_clause const& a, correlation_clause const& b) noexcept {
    return !(a == b);
}

::takatori::serializer::object_acceptor& operator<<(::takatori::serializer::object_acceptor& acceptor, correlation_clause const& value) {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor);
    property(acceptor, "correlation_name"sv, *value.correlation_name_);
    property(acceptor, "column_names"sv, *value.column_names_);
    region_property(acceptor, value);
    return acceptor;
}

std::ostream& operator<<(std::ostream& out, correlation_clause const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::table
