#include <mizugaki/ast/query/with_expression.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::query {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

using common::clone_vector;

with_expression::with_expression(
        bool_type is_recursive,
        common::vector<element_type> elements,
        unique_object_ptr<ast::query::expression> expression,
        region_type region) noexcept :
    super { region },
    is_recursive_ { is_recursive },
    elements_ { std::move(elements) },
    expression_ { std::move(expression) }
{}

with_expression::with_expression(
        std::initializer_list<element_type> elements,
        ast::query::expression&& expression,
        region_type region) :
    with_expression {
            false,
            decltype(elements_) { elements },
            clone_unique(std::move(expression)),
            region,
    }
{}

with_expression::with_expression(
        bool_type is_recursive,
        std::initializer_list<element_type> elements,
        ast::query::expression&& expression,
        region_type region) :
    with_expression {
            is_recursive,
            decltype(elements_) { elements },
            clone_unique(std::move(expression)),
            region,
    }
{}

with_expression::with_expression(with_expression const& other, object_creator creator) :
    with_expression {
            other.is_recursive_,
            clone_vector(other.elements_, creator),
            clone_unique(other.expression_, creator),
            other.region(),
    }
{}

with_expression::with_expression(with_expression&& other, object_creator creator) :
    with_expression {
            other.is_recursive_,
            clone_vector(other.elements_, creator),
            clone_unique(std::move(other.expression_), creator),
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

unique_object_ptr<ast::query::expression>& with_expression::expression() noexcept {
    return expression_;
}

unique_object_ptr<ast::query::expression> const& with_expression::expression() const noexcept {
    return expression_;
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
            && eq(a.expression_, b.expression_)
            && eq(a.is_recursive_, b.is_recursive_);
}

bool operator!=(with_expression const& a, with_expression const& b) noexcept {
    return !(a == b);
}

bool with_expression::equals(ast::query::expression const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void with_expression::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "is_recursive"sv, is_recursive_);
    property(acceptor, "elements"sv, elements_);
    property(acceptor, "expression"sv, expression_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, with_expression const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::query
