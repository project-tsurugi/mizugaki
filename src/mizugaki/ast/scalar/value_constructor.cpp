#include <mizugaki/ast/scalar/value_constructor.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::scalar {

using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

using common::clone_vector;

value_constructor::value_constructor(
        operator_kind_type operator_kind,
        common::vector<operand_type> elements,
        region_type region) noexcept:
    super { region },
    operator_kind_ { operator_kind },
    elements_ { std::move(elements) }
{}

value_constructor::value_constructor(value_constructor const& other, object_creator creator) :
    value_constructor {
            other.operator_kind_,
            clone_vector(other.elements_, creator),
            other.region(),
    }
{}

value_constructor::value_constructor(value_constructor&& other, object_creator creator) :
    value_constructor {
            other.operator_kind_,
            clone_vector(std::move(other.elements_), creator),
            other.region(),
    }
{}

value_constructor* value_constructor::clone(object_creator creator) const& {
    return creator.create_object<value_constructor>(*this, creator);
}

value_constructor* value_constructor::clone(object_creator creator) && {
    return creator.create_object<value_constructor>(std::move(*this), creator);
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

common::vector<expression::operand_type>& value_constructor::elements() noexcept {
    return elements_;
}

common::vector<expression::operand_type> const& value_constructor::elements() const noexcept {
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
            && *this == unsafe_downcast<value_constructor>(other);
}

} // namespace mizugaki::ast::scalar
