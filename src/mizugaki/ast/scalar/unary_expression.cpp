#include <mizugaki/ast/scalar/unary_expression.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::scalar {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

unary_expression::unary_expression(
        operator_kind_type operator_kind,
        operand_type operand,
        region_type region) noexcept :
    super { region },
    operator_kind_ { operator_kind },
    operand_ { std::move(operand) }
{}

unary_expression::unary_expression(
        operator_kind_type operator_kind,
        expression&& operand,
        region_type region) :
    unary_expression {
            operator_kind,
            clone_unique(std::move(operand)),
            region,
    }
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

bool operator==(unary_expression const& a, unary_expression const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return false;
    }
    return eq(a.operator_kind_, b.operator_kind_)
            && eq(a.operand_, b.operand_);
}

bool operator!=(unary_expression const& a, unary_expression const& b) noexcept {
    return !(a == b);
}

bool unary_expression::equals(expression const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void unary_expression::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "operator_kind"sv, operator_kind_);
    property(acceptor, "operand"sv, operand_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, unary_expression const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::scalar
