#include <mizugaki/ast/query/group_by_clause.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::query {


using common::clone_vector;
using common::to_vector;

group_by_clause::group_by_clause(
        std::vector<std::unique_ptr<element_type>> elements,
        region_type region) noexcept :
    element { region },
    elements_ { std::move(elements) }
{}

group_by_clause::group_by_clause(
        common::rvalue_list<element_type> elements,
        region_type region) :
    group_by_clause {
            to_vector(elements),
            region,
    }
{}

group_by_clause::group_by_clause(::takatori::util::clone_tag_t, group_by_clause const& other) :
    group_by_clause {
            clone_vector(*other.elements_),
            other.region(),
    }
{}

group_by_clause::group_by_clause(::takatori::util::clone_tag_t, group_by_clause&& other) :
    group_by_clause {
            clone_vector(std::move(*other.elements_)),
            other.region(),
    }
{}

std::vector<std::unique_ptr<group_by_clause::element_type>>& group_by_clause::elements() noexcept {
    return *elements_;
}

std::vector<std::unique_ptr<group_by_clause::element_type>> const& group_by_clause::elements() const noexcept {
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

::takatori::serializer::object_acceptor& operator<<(::takatori::serializer::object_acceptor& acceptor, group_by_clause const& value) {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor);
    property(acceptor, "elements"sv, *value.elements_);
    region_property(acceptor, value);
    return acceptor;
}

std::ostream& operator<<(std::ostream& out, group_by_clause const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::query
