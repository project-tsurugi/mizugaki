#include <mizugaki/ast/scalar/comparison_predicate.h>

#include <takatori/util/clonable.h>

namespace mizugaki::ast::scalar {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

comparison_predicate::comparison_predicate(
        operator_kind_type operator_kind,
        operand_type left,
        operand_type right,
        std::optional<quantifier_type> quantifier,
        region_type region) noexcept:
    super { region },
    operator_kind_ { operator_kind },
    left_ { std::move(left) },
    right_ { std::move(right) },
    quantifier_ { std::move(quantifier) }
{}

comparison_predicate::comparison_predicate(comparison_predicate const& other, object_creator creator) :
    comparison_predicate {
            other.operator_kind_,
            clone_unique(other.left_, creator),
            clone_unique(other.right_, creator),
            other.quantifier_,
            other.region(),
    }
{}

comparison_predicate::comparison_predicate(comparison_predicate&& other, object_creator creator) :
    comparison_predicate {
            other.operator_kind_,
            clone_unique(std::move(other.left_), creator),
            clone_unique(std::move(other.right_), creator),
            std::move(other.quantifier_),
            other.region(),
    }
{}

comparison_predicate* comparison_predicate::clone(object_creator creator) const& {
    return creator.create_object<comparison_predicate>(*this, creator);
}

comparison_predicate* comparison_predicate::clone(object_creator creator) && {
    return creator.create_object<comparison_predicate>(std::move(*this), creator);
}

expression::node_kind_type comparison_predicate::node_kind() const noexcept {
    return tag;
}

comparison_predicate::operator_kind_type& comparison_predicate::operator_kind() noexcept {
    return operator_kind_;
}

comparison_predicate::operator_kind_type const& comparison_predicate::operator_kind() const noexcept {
    return operator_kind_;
}

expression::operand_type& comparison_predicate::left() noexcept {
    return left_;
}

expression::operand_type const& comparison_predicate::left() const noexcept {
    return left_;
}

expression::operand_type& comparison_predicate::right() noexcept {
    return right_;
}

expression::operand_type const& comparison_predicate::right() const noexcept {
    return right_;
}

std::optional<comparison_predicate::quantifier_type>& comparison_predicate::quantifier() noexcept {
    return quantifier_;
}

std::optional<comparison_predicate::quantifier_type> const& comparison_predicate::quantifier() const noexcept {
    return quantifier_;
}

} // namespace mizugaki::ast::scalar
