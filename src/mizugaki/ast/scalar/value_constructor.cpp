#include <mizugaki/ast/scalar/value_constructor.h>

namespace mizugaki::ast::scalar {

using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

using common::clone_vector;

value_constructor::value_constructor(
        constructor_kind_type constructor_kind,
        common::vector<operand_type> elements,
        region_type region) noexcept:
    super { region },
    constructor_kind_ { constructor_kind },
    elements_ { std::move(elements) }
{}

value_constructor::value_constructor(value_constructor const& other, object_creator creator) :
    value_constructor {
            other.constructor_kind_,
            clone_vector(other.elements_, creator),
            other.region(),
    }
{}

value_constructor::value_constructor(value_constructor&& other, object_creator creator) :
    value_constructor {
            other.constructor_kind_,
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

value_constructor::constructor_kind_type& value_constructor::constructor_kind() noexcept {
    return constructor_kind_;
}

value_constructor::constructor_kind_type const& value_constructor::constructor_kind() const noexcept {
    return constructor_kind_;
}

common::vector<expression::operand_type>& value_constructor::elements() noexcept {
    return elements_;
}

common::vector<expression::operand_type> const& value_constructor::elements() const noexcept {
    return elements_;
}

} // namespace mizugaki::ast::scalar
