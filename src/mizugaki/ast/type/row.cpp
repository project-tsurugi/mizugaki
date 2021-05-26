#include <mizugaki/ast/type/row.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::type {


using common::clone_vector;

row::row(
        std::vector<field_definition> elements,
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

row::row(::takatori::util::clone_tag_t, row const& other) :
    row {
            clone_vector(other.elements_),
            other.region(),
    }
{}

row::row(::takatori::util::clone_tag_t, row&& other) :
    row {
            clone_vector(std::move(other.elements_)),
            other.region(),
    }
{}

row* row::clone() const& {
    return new row(::takatori::util::clone_tag, *this); // NOLINT
}

row* row::clone() && {
    return new row(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

type::node_kind_type row::node_kind() const noexcept {
    return tag;
}

std::vector<field_definition>& row::elements() noexcept {
    return elements_;
}

std::vector<field_definition> const& row::elements() const noexcept {
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
