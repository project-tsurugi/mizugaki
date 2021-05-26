#include <mizugaki/ast/scalar/cast_expression.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::scalar {

using operator_kind_type = cast_expression::operator_kind_type;

using ::takatori::util::clone_unique;

cast_expression::cast_expression(
        operator_kind_type operator_kind,
        operand_type operand,
        std::unique_ptr<type::type> type,
        region_type region) noexcept :
    super { region },
    operator_kind_ { operator_kind },
    operand_ { std::move(operand) },
    type_ { std::move(type) }
{}

cast_expression::cast_expression(
        operator_kind_type operator_kind,
        expression&& operand,
        type::type&& type,
        region_type region) :
    cast_expression {
            operator_kind,
            clone_unique(std::move(operand)),
            clone_unique(std::move(type)),
            region,
    }
{}

cast_expression::cast_expression(::takatori::util::clone_tag_t, cast_expression const& other) :
    cast_expression {
            other.operator_kind_,
            clone_unique(other.operand_),
            clone_unique(other.type_),
            other.region(),
    }
{}

cast_expression::cast_expression(::takatori::util::clone_tag_t, cast_expression&& other) :
    cast_expression {
            other.operator_kind_,
            clone_unique(std::move(other.operand_)),
            clone_unique(std::move(other.type_)),
            other.region(),
    }
{}

cast_expression* cast_expression::clone() const& {
    return new cast_expression(::takatori::util::clone_tag, *this); // NOLINT
}

cast_expression* cast_expression::clone() && {
    return new cast_expression(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
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

std::unique_ptr<type::type>& cast_expression::type() noexcept {
    return type_;
}

std::unique_ptr<type::type> const& cast_expression::type() const noexcept {
    return type_;
}

bool operator==(cast_expression const& a, cast_expression const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return false;
    }
    return eq(a.operator_kind_, b.operator_kind_)
            && eq(a.operand_, b.operand_)
            && eq(a.type_, b.type_);
}

bool operator!=(cast_expression const& a, cast_expression const& b) noexcept {
    return !(a == b);
}

bool cast_expression::equals(expression const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void cast_expression::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "operator_kind"sv, operator_kind_);
    property(acceptor, "operand"sv, operand_);
    property(acceptor, "type"sv, type_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, cast_expression const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::scalar
