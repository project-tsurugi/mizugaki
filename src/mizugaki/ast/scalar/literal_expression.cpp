#include <mizugaki/ast/scalar/literal_expression.h>

#include <takatori/util/clonable.h>

namespace mizugaki::ast::scalar {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

literal_expression::literal_expression(
        unique_object_ptr<literal::literal> value,
        region_type region) noexcept:
    super { region },
    value_ { std::move(value) }
{}

literal_expression::literal_expression(literal_expression const& other, object_creator creator) :
    literal_expression {
            clone_unique(other.value_, creator),
            other.region(),
    }
{}

literal_expression::literal_expression(literal_expression&& other, object_creator creator) :
    literal_expression {
            clone_unique(std::move(other.value_), creator),
            other.region(),
    }
{}

literal_expression* literal_expression::clone(object_creator creator) const& {
    return creator.create_object<literal_expression>(*this, creator);
}

literal_expression* literal_expression::clone(object_creator creator) && {
    return creator.create_object<literal_expression>(std::move(*this), creator);
}

expression::node_kind_type literal_expression::node_kind() const noexcept {
    return tag;
}

unique_object_ptr<literal::literal>& literal_expression::value() noexcept {
    return value_;
}

unique_object_ptr<literal::literal> const& literal_expression::value() const noexcept {
    return value_;
}

} // namespace mizugaki::ast::scalar