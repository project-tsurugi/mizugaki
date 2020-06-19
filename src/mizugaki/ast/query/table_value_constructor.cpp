#include <mizugaki/ast/query/table_value_constructor.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::query {

using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

using common::clone_vector;
using common::to_vector;

table_value_constructor::table_value_constructor(
        common::vector<unique_object_ptr<scalar::expression>> elements,
        region_type region) noexcept :
    super { region },
    elements_ { std::move(elements) }
{}

table_value_constructor::table_value_constructor(
        common::rvalue_list<scalar::expression> elements,
        element::region_type region) noexcept :
    table_value_constructor {
            to_vector(elements),
            region,
    }
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

bool operator==(table_value_constructor const& a, table_value_constructor const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return false;
    }
    return eq(a.elements_, b.elements_);
}

bool operator!=(table_value_constructor const& a, table_value_constructor const& b) noexcept {
    return !(a == b);
}

bool table_value_constructor::equals(expression const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void table_value_constructor::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "elements"sv, elements_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, table_value_constructor const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::query
