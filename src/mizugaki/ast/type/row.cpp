#include <mizugaki/ast/type/row.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::type {

using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

using common::clone_vector;

row::row(
        common::vector<field_definition> elements,
        region_type region) noexcept:
    super { region },
    elements_ { std::move(elements) }
{}

row::row(
        std::initializer_list<field_definition> elements,
        region_type region) :
    row {
            decltype(elements_) { elements },
            region,
    }
{}

row::row(row const& other, object_creator creator) :
    row {
            clone_vector(other.elements_, creator),
            other.region(),
    }
{}

row::row(row&& other, object_creator creator) :
    row {
            clone_vector(std::move(other.elements_), creator),
            other.region(),
    }
{}

row* row::clone(object_creator creator) const& {
    return creator.create_object<row>(*this, creator);
}

row* row::clone(object_creator creator) && {
    return creator.create_object<row>(std::move(*this), creator);
}

type::node_kind_type row::node_kind() const noexcept {
    return tag;
}

common::vector<field_definition>& row::elements() noexcept {
    return elements_;
}

common::vector<field_definition> const& row::elements() const noexcept {
    return elements_;
}

bool operator==(row const& a, row const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return false;
    }
    return eq(a.elements_, b.elements_);
}

bool operator!=(row const& a, row const& b) noexcept {
    return !(a == b);
}

bool row::equals(type const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void row::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "elements"sv, elements_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, row const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::type
