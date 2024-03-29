#include <mizugaki/ast/query/with_element.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::query {

using ::takatori::util::clone_unique;

using common::clone_vector;
using common::to_vector;

with_element::with_element(
        std::unique_ptr<name::simple> name,
        std::vector<std::unique_ptr<name::simple>> column_names,
        std::unique_ptr<ast::query::expression> expression,
        region_type region) noexcept :
    element { region },
    name_ { std::move(name) },
    column_names_ { std::move(column_names) },
    expression_ { std::move(expression) }
{}


with_element::with_element(
        name::simple&& name,
        ast::query::expression&& expression,
        region_type region) :
    with_element {
            clone_unique(std::move(name)),
            {},
            clone_unique(std::move(expression)),
            region,
    }
{}

with_element::with_element(
        name::simple&& name,
        common::rvalue_list<name::simple> column_names,
        ast::query::expression&& expression,
        element::region_type region) :
    with_element {
            clone_unique(std::move(name)),
            to_vector(column_names),
            clone_unique(std::move(expression)),
            region,
    }
{}

with_element::with_element(::takatori::util::clone_tag_t, with_element const& other) :
    with_element {
            clone_unique(*other.name_),
            clone_vector(*other.column_names_),
            clone_unique(*other.expression_),
            other.region(),
    }
{}

with_element::with_element(::takatori::util::clone_tag_t, with_element&& other) :
    with_element {
            clone_unique(std::move(*other.name_)),
            clone_vector(std::move(*other.column_names_)),
            clone_unique(std::move(*other.expression_)),
            other.region(),
    }
{}

std::unique_ptr<name::simple>& with_element::name() noexcept {
    return *name_;
}

std::unique_ptr<name::simple> const& with_element::name() const noexcept {
    return *name_;
}

std::unique_ptr<ast::query::expression>& with_element::expression() noexcept {
    return *expression_;
}

std::unique_ptr<ast::query::expression> const& with_element::expression() const noexcept {
    return *expression_;
}

std::vector<std::unique_ptr<name::simple>>& with_element::column_names() noexcept {
    return *column_names_;
}

std::vector<std::unique_ptr<name::simple>> const& with_element::column_names() const noexcept {
    return *column_names_;
}

bool operator==(with_element const& a, with_element const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return false;
    }
    return eq(*a.name_, *b.name_)
            && eq(*a.expression_, *b.expression_)
            && eq(*a.column_names_, *b.column_names_);
}

bool operator!=(with_element const& a, with_element const& b) noexcept {
    return !(a == b);
}

::takatori::serializer::object_acceptor& operator<<(::takatori::serializer::object_acceptor& acceptor, with_element const& value) {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor);
    property(acceptor, "name"sv, *value.name_);
    property(acceptor, "expression"sv, *value.expression_);
    property(acceptor, "column_names"sv, *value.column_names_);
    region_property(acceptor, value);
    return acceptor;
}

std::ostream& operator<<(std::ostream& out, with_element const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::query
