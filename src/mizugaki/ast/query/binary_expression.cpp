#include <mizugaki/ast/query/binary_expression.h>

namespace mizugaki::ast::query {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

binary_expression::binary_expression(
        operator_kind_type operator_kind,
        unique_object_ptr<expression> left,
        unique_object_ptr<expression> right,
        std::optional<quantifier_type> quantifier,
        std::optional<corresponding_type> corresponding,
        region_type region) noexcept :
    super { region },
    operator_kind_ { operator_kind },
    left_ { std::move(left) },
    right_ { std::move(right) },
    quantifier_ { std::move(quantifier) },
    corresponding_ { std::move(corresponding) }
{}

binary_expression::binary_expression(binary_expression const& other, object_creator creator) :
    binary_expression {
            other.operator_kind_,
            clone_unique(other.left_, creator),
            clone_unique(other.right_, creator),
            other.quantifier_,
            other.corresponding_,
            other.region(),
    }
{}

binary_expression::binary_expression(binary_expression&& other, object_creator creator) :
    binary_expression {
            other.operator_kind_,
            clone_unique(std::move(other.left_), creator),
            clone_unique(std::move(other.right_), creator),
            std::move(other.quantifier_),
            std::move(other.corresponding_),
            other.region(),
    }
{}

binary_expression* binary_expression::clone(object_creator creator) const& {
    return creator.create_object<binary_expression>(*this, creator);
}

binary_expression* binary_expression::clone(object_creator creator)&& {
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

unique_object_ptr<expression>& binary_expression::left() noexcept {
    return left_;
}

unique_object_ptr<expression> const& binary_expression::left() const noexcept {
    return left_;
}

unique_object_ptr<expression>& binary_expression::right() noexcept {
    return right_;
}

unique_object_ptr<expression> const& binary_expression::right() const noexcept {
    return right_;
}

std::optional<binary_expression::quantifier_type>& binary_expression::quantifier() noexcept {
    return quantifier_;
}

std::optional<binary_expression::quantifier_type> const& binary_expression::quantifier() const noexcept {
    return quantifier_;
}

std::optional<binary_expression::corresponding_type>& binary_expression::corresponding() noexcept {
    return corresponding_;
}

std::optional<binary_expression::corresponding_type> const& binary_expression::corresponding() const noexcept {
    return corresponding_;
}

} // namespace mizugaki::ast::query
