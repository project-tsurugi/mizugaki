#include <mizugaki/ast/query/with_expression.h>

#include <takatori/util/clonable.h>

namespace mizugaki::ast::query {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

using common::clone_vector;

with_expression::with_expression(
        common::vector<element_type> elements,
        unique_object_ptr<expression> body,
        region_type region) noexcept :
    super { region },
    elements_ { std::move(elements) },
    body_ { std::move(body) }
{}

with_expression::with_expression(with_expression const& other, object_creator creator) :
    with_expression {
            clone_vector(other.elements_, creator),
            clone_unique(other.body_, creator),
            other.region(),
    }
{}

with_expression::with_expression(with_expression&& other, object_creator creator) :
    with_expression {
            clone_vector(other.elements_, creator),
            clone_unique(std::move(other.body_), creator),
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

} // namespace mizugaki::ast::query
