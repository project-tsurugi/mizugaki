#include <mizugaki/ast/scalar/binary_expression.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::scalar {

using ::takatori::util::clone_unique;

binary_expression::binary_expression(
        operand_type left,
        operator_kind_type operator_kind,
        operand_type right,
        region_type region) noexcept :
    super { region },
    left_ { std::move(left) },
    operator_kind_ { operator_kind },
    right_ { std::move(right) }
{}

binary_expression::binary_expression(
        expression&& left,
        operator_kind_type operator_kind,
        expression&& right,
        region_type region) :
    binary_expression {
            clone_unique(std::move(left)),
            operator_kind,
            clone_unique(std::move(right)),
            region,
    }
{}

binary_expression::binary_expression(::takatori::util::clone_tag_t, binary_expression const& other) :
    binary_expression {
            clone_unique(other.left_),
            other.operator_kind_,
            clone_unique(other.right_),
            other.region(),
    }
{}

binary_expression::binary_expression(::takatori::util::clone_tag_t, binary_expression&& other) :
    binary_expression {
            clone_unique(std::move(other.left_)),
            other.operator_kind_,
            clone_unique(std::move(other.right_)),
            other.region(),
    }
{}

binary_expression* binary_expression::clone() const& {
    return new binary_expression(::takatori::util::clone_tag, *this); // NOLINT
}

binary_expression* binary_expression::clone() && {
    return new binary_expression(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

expression::node_kind_type binary_expression::node_kind() const noexcept {
    return tag;
}

binary_expression::operator_kind_type& binary_expression::operator_kind() noexcept {
    return operator_kind_;
}

binary_expression::operator_kind_type const& binary_expression::operator_kind() const noexcept {
    return operator_kind_;
}

expression::operand_type& binary_expression::left() noexcept {
    return left_;
}

expression::operand_type const& binary_expression::left() const noexcept {
    return left_;
}

expression::operand_type& binary_expression::right() noexcept {
    return right_;
}

expression::operand_type const& binary_expression::right() const noexcept {
    return right_;
}

bool operator==(binary_expression const& a, binary_expression const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return false;
    }
    return eq(a.operator_kind_, b.operator_kind_)
            && eq(a.left_, b.left_)
            && eq(a.right_, b.right_);
}

bool operator!=(binary_expression const& a, binary_expression const& b) noexcept {
    return !(a == b);
}

bool binary_expression::equals(expression const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void binary_expression::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "left"sv, left_);
    property(acceptor, "operator_kind"sv, operator_kind_);
    property(acceptor, "right"sv, right_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, binary_expression const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::scalar
