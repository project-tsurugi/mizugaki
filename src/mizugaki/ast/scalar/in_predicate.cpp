#include <mizugaki/ast/scalar/in_predicate.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::scalar {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

using common::clone_vector;

in_predicate::in_predicate(
        operand_type left,
        common::vector<operand_type> right,
        not_type is_not,
        region_type region) noexcept :
    super { region },
    left_ { std::move(left) },
    right_ { std::move(right) },
    is_not_ { is_not }
{}

in_predicate::in_predicate(in_predicate const& other, object_creator creator) :
    in_predicate {
            clone_unique(other.left_, creator),
            clone_vector(other.right_, creator),
            other.is_not_,
            other.region(),
    }
{}

in_predicate::in_predicate(in_predicate&& other, object_creator creator) :
    in_predicate {
            clone_unique(std::move(other.left_), creator),
            clone_vector(std::move(other.right_), creator),
            other.is_not_,
            other.region(),
    }
{}

in_predicate* in_predicate::clone(object_creator creator) const& {
    return creator.create_object<in_predicate>(*this, creator);
}

in_predicate* in_predicate::clone(object_creator creator) && {
    return creator.create_object<in_predicate>(std::move(*this), creator);
}

expression::node_kind_type in_predicate::node_kind() const noexcept {
    return tag;
}

expression::operand_type& in_predicate::left() noexcept {
    return left_;
}

expression::operand_type const& in_predicate::left() const noexcept {
    return left_;
}

common::vector<expression::operand_type>& in_predicate::right() noexcept {
    return right_;
}

common::vector<expression::operand_type> const& in_predicate::right() const noexcept {
    return right_;
}

in_predicate::not_type& in_predicate::is_not() noexcept {
    return is_not_;
}

in_predicate::not_type const& in_predicate::is_not() const noexcept {
    return is_not_;
}

bool operator==(in_predicate const& a, in_predicate const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return false;
    }
    return eq(a.left_, b.left_)
            && eq(a.right_, b.right_)
            && eq(a.is_not_, b.is_not_);
}

bool operator!=(in_predicate const& a, in_predicate const& b) noexcept {
    return !(a == b);
}

bool in_predicate::equals(expression const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<in_predicate>(other);
}

} // namespace mizugaki::ast::scalar
