#include <mizugaki/ast/scalar/binary_expression.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::scalar {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

binary_expression::binary_expression(
        operator_kind_type operator_kind,
        operand_type left,
        operand_type right,
        region_type region) noexcept:
    super { region },
    operator_kind_ { operator_kind },
    left_ { std::move(left) },
    right_ { std::move(right) }
{}

binary_expression::binary_expression(binary_expression const& other, object_creator creator) :
    binary_expression {
            other.operator_kind_,
            clone_unique(other.left_, creator),
            clone_unique(other.right_, creator),
            other.region(),
    }
{}

binary_expression::binary_expression(binary_expression&& other, object_creator creator) :
    binary_expression {
            other.operator_kind_,
            clone_unique(std::move(other.left_), creator),
            clone_unique(std::move(other.right_), creator),
            other.region(),
    }
{}

binary_expression* binary_expression::clone(object_creator creator) const& {
    return creator.create_object<binary_expression>(*this, creator);
}

binary_expression* binary_expression::clone(object_creator creator) && {
    return creator.create_object<binary_expression>(std::move(*this), creator);
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
            && *this == unsafe_downcast<binary_expression>(other);
}

} // namespace mizugaki::ast::scalar
