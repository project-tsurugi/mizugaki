#include <mizugaki/ast/scalar/between_predicate.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::scalar {

using operator_kind_type = between_predicate::operator_kind_type;

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;

between_predicate::between_predicate(
        operand_type target,
        operand_type left,
        operand_type right,
        std::optional<operator_kind_type> operator_kind,
        not_type is_not,
        region_type region) noexcept:
    super { region },
    target_ { std::move(target) },
    left_ { std::move(left) },
    right_ { std::move(right) },
    operator_kind_ { std::move(operator_kind) },
    is_not_ { is_not }
{}

between_predicate::between_predicate(between_predicate const& other, object_creator creator) :
    between_predicate {
            clone_unique(other.target_, creator),
            clone_unique(other.left_, creator),
            clone_unique(other.right_, creator),
            other.operator_kind_,
            other.is_not_,
            other.region(),
    }
{}

between_predicate::between_predicate(between_predicate&& other, object_creator creator) :
    between_predicate {
            clone_unique(std::move(other.target_), creator),
            clone_unique(std::move(other.left_), creator),
            clone_unique(std::move(other.right_), creator),
            other.operator_kind_,
            other.is_not_,
            other.region(),
    }
{}

between_predicate* between_predicate::clone(object_creator creator) const& {
    return creator.create_object<between_predicate>(*this, creator);
}

between_predicate* between_predicate::clone(object_creator creator) && {
    return creator.create_object<between_predicate>(std::move(*this), creator);
}

expression::node_kind_type between_predicate::node_kind() const noexcept {
    return tag;
}

expression::operand_type& between_predicate::target() noexcept {
    return target_;
}

expression::operand_type const& between_predicate::target() const noexcept {
    return target_;
}

expression::operand_type& between_predicate::left() noexcept {
    return left_;
}

expression::operand_type const& between_predicate::left() const noexcept {
    return left_;
}

expression::operand_type& between_predicate::right() noexcept {
    return right_;
}

expression::operand_type const& between_predicate::right() const noexcept {
    return right_;
}

std::optional<operator_kind_type>& between_predicate::operator_kind() noexcept {
    return operator_kind_;
}

std::optional<operator_kind_type> const& between_predicate::operator_kind() const noexcept {
    return operator_kind_;
}

between_predicate::not_type& between_predicate::is_not() noexcept {
    return is_not_;
}

between_predicate::not_type const& between_predicate::is_not() const noexcept {
    return is_not_;
}

bool operator==(between_predicate const& a, between_predicate const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return false;
    }
    return eq(a.target_, b.target_)
            && eq(a.left_, b.left_)
            && eq(a.right_, b.right_)
            && eq(a.operator_kind_, b.operator_kind_)
            && eq(a.is_not_, b.is_not_);
}

bool operator!=(between_predicate const& a, between_predicate const& b) noexcept {
    return !(a == b);
}

bool between_predicate::equals(expression const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<between_predicate>(other);
}

} // namespace mizugaki::ast::scalar
