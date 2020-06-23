#include <mizugaki/ast/query/corresponding_clause.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::query {

using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

using common::clone_vector;
using common::to_vector;

corresponding_clause::corresponding_clause(
        common::vector<unique_object_ptr<name::simple>> column_names,
        region_type region) noexcept :
    element { region },
    column_names_ { std::move(column_names) }
{}


corresponding_clause::corresponding_clause(
        common::rvalue_list<name::simple> column_names,
        region_type region) :
    corresponding_clause {
            to_vector(column_names),
            region,
    }
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

bool operator==(corresponding_clause const& a, corresponding_clause const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return false;
    }
    return eq(*a.column_names_, *b.column_names_);
}

bool operator!=(corresponding_clause const& a, corresponding_clause const& b) noexcept {
    return !(a == b);
}

::takatori::serializer::object_acceptor& operator<<(::takatori::serializer::object_acceptor& acceptor, corresponding_clause const& value) {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor);
    property(acceptor, "column_names"sv, *value.column_names_);
    region_property(acceptor, value);
    return acceptor;
}

std::ostream& operator<<(std::ostream& out, corresponding_clause const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::query
