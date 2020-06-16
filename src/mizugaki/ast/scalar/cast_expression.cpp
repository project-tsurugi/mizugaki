#include <mizugaki/ast/scalar/cast_expression.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::scalar {

using operator_kind_type = cast_expression::operator_kind_type;

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

cast_expression::cast_expression(
        operator_kind_type operator_kind,
        operand_type operand,
        unique_object_ptr<type::type> target,
        region_type region) noexcept:
    super { region },
    operator_kind_ { operator_kind },
    operand_ { std::move(operand) },
    target_ { std::move(target) }
{}

cast_expression::cast_expression(cast_expression const& other, object_creator creator) :
    cast_expression {
            other.operator_kind_,
            clone_unique(other.operand_, creator),
            clone_unique(other.target_, creator),
            other.region(),
    }
{}

cast_expression::cast_expression(cast_expression&& other, object_creator creator) :
    cast_expression {
            other.operator_kind_,
            clone_unique(std::move(other.operand_), creator),
            clone_unique(std::move(other.target_), creator),
            other.region(),
    }
{}

cast_expression* cast_expression::clone(object_creator creator) const& {
    return creator.create_object<cast_expression>(*this, creator);
}

cast_expression* cast_expression::clone(object_creator creator) && {
    return creator.create_object<cast_expression>(std::move(*this), creator);
}

expression::node_kind_type cast_expression::node_kind() const noexcept {
    return tag;
}

operator_kind_type& cast_expression::operator_kind() noexcept {
    return operator_kind_;
}

operator_kind_type const& cast_expression::operator_kind() const noexcept {
    return operator_kind_;
}

expression::operand_type& cast_expression::operand() noexcept {
    return operand_;
}

expression::operand_type const& cast_expression::operand() const noexcept {
    return operand_;
}

unique_object_ptr<type::type>& cast_expression::target() noexcept {
    return target_;
}

unique_object_ptr<type::type> const& cast_expression::target() const noexcept {
    return target_;
}

bool operator==(cast_expression const& a, cast_expression const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return false;
    }
    return eq(a.operator_kind_, b.operator_kind_)
            && eq(a.operand_, b.operand_)
            && eq(a.target_, b.target_);
}

bool operator!=(cast_expression const& a, cast_expression const& b) noexcept {
    return !(a == b);
}

bool cast_expression::equals(expression const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

} // namespace mizugaki::ast::scalar
