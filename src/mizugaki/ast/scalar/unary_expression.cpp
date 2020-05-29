#include <mizugaki/ast/scalar/unary_expression.h>

#include <takatori/util/clonable.h>

namespace mizugaki::ast::scalar {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

unary_expression::unary_expression(
        operator_kind_type operator_kind,
        operand_type operand,
        region_type region) noexcept:
    super { region },
    operator_kind_ { operator_kind },
    operand_ { std::move(operand) }
{}

unary_expression::unary_expression(unary_expression const& other, object_creator creator) :
    unary_expression {
            other.operator_kind_,
            clone_unique(other.operand_, creator),
            other.region(),
    }
{}

unary_expression::unary_expression(unary_expression&& other, object_creator creator) :
    unary_expression {
            other.operator_kind_,
            clone_unique(std::move(other.operand_), creator),
            other.region(),
    }
{}

unary_expression* unary_expression::clone(object_creator creator) const& {
    return creator.create_object<unary_expression>(*this, creator);
}

unary_expression* unary_expression::clone(object_creator creator) && {
    return creator.create_object<unary_expression>(std::move(*this), creator);
}

expression::node_kind_type unary_expression::node_kind() const noexcept {
    return tag;
}

unary_expression::operator_kind_type& unary_expression::operator_kind() noexcept {
    return operator_kind_;
}

unary_expression::operator_kind_type const& unary_expression::operator_kind() const noexcept {
    return operator_kind_;
}

expression::operand_type& unary_expression::operand() noexcept {
    return operand_;
}

expression::operand_type const& unary_expression::operand() const noexcept {
    return operand_;
}

} // namespace mizugaki::ast::scalar
