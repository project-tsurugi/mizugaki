#include <mizugaki/ast/query/with_expression.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::query {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

using common::clone_vector;

with_expression::with_expression(
        common::vector<element_type> elements,
        unique_object_ptr<expression> body,
        bool_type is_recursive,
        region_type region) noexcept :
    super { region },
    elements_ { std::move(elements) },
    body_ { std::move(body) },
    is_recursive_ { is_recursive }
{}

with_expression::with_expression(with_expression const& other, object_creator creator) :
    with_expression {
            clone_vector(other.elements_, creator),
            clone_unique(other.body_, creator),
            other.is_recursive_,
            other.region(),
    }
{}

with_expression::with_expression(with_expression&& other, object_creator creator) :
    with_expression {
            clone_vector(other.elements_, creator),
            clone_unique(std::move(other.body_), creator),
            other.is_recursive_,
            other.region(),
    }
{}

with_expression* with_expression::clone(object_creator creator) const& {
    return creator.create_object<with_expression>(*this, creator);
}

with_expression* with_expression::clone(object_creator creator)&& {
    return creator.create_object<with_expression>(std::move(*this), creator);
}

expression::node_kind_type with_expression::node_kind() const noexcept {
    return tag;
}

common::vector<with_expression::element_type>& with_expression::elements() noexcept {
    return elements_;
}

common::vector<with_expression::element_type> const& with_expression::elements() const noexcept {
    return elements_;
}

unique_object_ptr<expression>& with_expression::body() noexcept {
    return body_;
}

unique_object_ptr<expression> const& with_expression::body() const noexcept {
    return body_;
}

with_expression::bool_type& with_expression::is_recursive() noexcept {
    return is_recursive_;
}

with_expression::bool_type const& with_expression::is_recursive() const noexcept {
    return is_recursive_;
}

bool operator==(with_expression const& a, with_expression const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return false;
    }
    return eq(a.elements_, b.elements_)
            && eq(a.body_, b.body_)
            && eq(a.is_recursive_, b.is_recursive_);
}

bool operator!=(with_expression const& a, with_expression const& b) noexcept {
    return !(a == b);
}

bool with_expression::equals(expression const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

} // namespace mizugaki::ast::query
