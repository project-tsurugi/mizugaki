#include <mizugaki/ast/scalar/convert_expression.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::scalar {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

convert_expression::convert_expression(
        operator_kind_type operator_kind,
        operand_type operand,
        unique_object_ptr<name::name> name,
        region_type region) noexcept:
    super { region },
    operator_kind_ { operator_kind },
    operand_ { std::move(operand) },
    name_ { std::move(name) }
{}

convert_expression::convert_expression(convert_expression const& other, object_creator creator) :
    convert_expression {
            other.operator_kind_,
            clone_unique(other.operand_, creator),
            clone_unique(other.name_, creator),
            other.region(),
    }
{}

convert_expression::convert_expression(convert_expression&& other, object_creator creator) :
    convert_expression {
            other.operator_kind_,
            clone_unique(std::move(other.operand_), creator),
            clone_unique(std::move(other.name_), creator),
            other.region(),
    }
{}

convert_expression* convert_expression::clone(object_creator creator) const& {
    return creator.create_object<convert_expression>(*this, creator);
}

convert_expression* convert_expression::clone(object_creator creator) && {
    return creator.create_object<convert_expression>(std::move(*this), creator);
}

expression::node_kind_type convert_expression::node_kind() const noexcept {
    return tag;
}

convert_expression::operator_kind_type& convert_expression::operator_kind() noexcept {
    return operator_kind_;
}

convert_expression::operator_kind_type const& convert_expression::operator_kind() const noexcept {
    return operator_kind_;
}

expression::operand_type& convert_expression::operand() noexcept {
    return operand_;
}

expression::operand_type const& convert_expression::operand() const noexcept {
    return operand_;
}

unique_object_ptr<name::name>& convert_expression::name() noexcept {
    return name_;
}

unique_object_ptr<name::name> const& convert_expression::name() const noexcept {
    return name_;
}

bool operator==(convert_expression const& a, convert_expression const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return false;
    }
    return eq(a.operator_kind_, b.operator_kind_)
            && eq(a.operand_, b.operand_)
            && eq(a.name_, b.name_);
}

bool operator!=(convert_expression const& a, convert_expression const& b) noexcept {
    return !(a == b);
}

bool convert_expression::equals(expression const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void convert_expression::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "operand"sv, operand_);
    property(acceptor, "operator_kind"sv, operator_kind_);
    property(acceptor, "name"sv, name_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, convert_expression const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::scalar
