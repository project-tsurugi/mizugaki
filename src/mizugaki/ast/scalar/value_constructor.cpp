#include <mizugaki/ast/scalar/value_constructor.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::scalar {


using common::clone_vector;
using common::to_vector;

value_constructor::value_constructor(
        operator_kind_type operator_kind,
        std::vector<operand_type> elements,
        region_type region) noexcept:
    super { region },
    operator_kind_ { operator_kind },
    elements_ { std::move(elements) }
{}

value_constructor::value_constructor(
        common::rvalue_list<scalar::expression> elements,
        region_type region) :
    value_constructor {
            value_constructor_kind::row,
            to_vector(elements),
            region,
    }
{}

value_constructor::value_constructor(
        value_constructor_kind operator_kind,
        common::rvalue_list<scalar::expression> elements,
        region_type region) :
    value_constructor {
            operator_kind,
            to_vector(elements),
            region,
    }
{}

value_constructor::value_constructor(::takatori::util::clone_tag_t, value_constructor const& other) :
    value_constructor {
            other.operator_kind_,
            clone_vector(other.elements_),
            other.region(),
    }
{}

value_constructor::value_constructor(::takatori::util::clone_tag_t, value_constructor&& other) :
    value_constructor {
            other.operator_kind_,
            clone_vector(std::move(other.elements_)),
            other.region(),
    }
{}

value_constructor* value_constructor::clone() const& {
    return new value_constructor(::takatori::util::clone_tag, *this); // NOLINT
}

value_constructor* value_constructor::clone() && {
    return new value_constructor(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

expression::node_kind_type value_constructor::node_kind() const noexcept {
    return tag;
}

value_constructor::operator_kind_type& value_constructor::operator_kind() noexcept {
    return operator_kind_;
}

value_constructor::operator_kind_type const& value_constructor::operator_kind() const noexcept {
    return operator_kind_;
}

std::vector<expression::operand_type>& value_constructor::elements() noexcept {
    return elements_;
}

std::vector<expression::operand_type> const& value_constructor::elements() const noexcept {
    return elements_;
}

bool operator==(value_constructor const& a, value_constructor const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return false;
    }
    return eq(a.operator_kind_, b.operator_kind_)
            && eq(a.elements_, b.elements_);
}

bool operator!=(value_constructor const& a, value_constructor const& b) noexcept {
    return !(a == b);
}

bool value_constructor::equals(expression const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void value_constructor::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "operator_kind"sv, operator_kind_);
    property(acceptor, "elements"sv, elements_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, value_constructor const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::scalar
