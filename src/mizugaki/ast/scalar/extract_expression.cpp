#include <mizugaki/ast/scalar/extract_expression.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::scalar {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

extract_expression::extract_expression(
        field_type field,
        operand_type operand,
        region_type region) noexcept:
    super { region },
    field_ { field },
    operand_ { std::move(operand) }
{}

extract_expression::extract_expression(extract_expression const& other, object_creator creator) :
    extract_expression {
            other.field_,
            clone_unique(other.operand_, creator),
            other.region(),
    }
{}

extract_expression::extract_expression(extract_expression&& other, object_creator creator) :
    extract_expression {
            other.field_,
            clone_unique(std::move(other.operand_), creator),
            other.region(),
    }
{}

extract_expression* extract_expression::clone(object_creator creator) const& {
    return creator.create_object<extract_expression>(*this, creator);
}

extract_expression* extract_expression::clone(object_creator creator) && {
    return creator.create_object<extract_expression>(std::move(*this), creator);
}

expression::node_kind_type extract_expression::node_kind() const noexcept {
    return tag;
}

extract_expression::field_type& extract_expression::field() noexcept {
    return field_;
}

extract_expression::field_type const& extract_expression::field() const noexcept {
    return field_;
}

expression::operand_type& extract_expression::operand() noexcept {
    return operand_;
}

expression::operand_type const& extract_expression::operand() const noexcept {
    return operand_;
}

bool operator==(extract_expression const& a, extract_expression const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return false;
    }
    return eq(a.field_, b.field_)
            && eq(a.operand_, b.operand_);
}

bool operator!=(extract_expression const& a, extract_expression const& b) noexcept {
    return !(a == b);
}

bool extract_expression::equals(expression const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void extract_expression::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "field"sv, field_);
    property(acceptor, "operand"sv, operand_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, extract_expression const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::scalar
