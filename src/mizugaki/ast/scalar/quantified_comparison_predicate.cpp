#include <mizugaki/ast/scalar/quantified_comparison_predicate.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::scalar {

using ::takatori::util::clone_unique;

quantified_comparison_predicate::quantified_comparison_predicate(
        operand_type left,
        operator_kind_type operator_kind,
        quantifier_type quantifier,
        std::unique_ptr<query::expression> right,
        region_type region) noexcept:
    super { region },
    left_ { std::move(left) },
    operator_kind_ { operator_kind },
    quantifier_ { quantifier },
    right_ { std::move(right) }
{}

quantified_comparison_predicate::quantified_comparison_predicate(
        expression&& left,
        operator_kind_type operator_kind,
        quantifier_type quantifier,
        query::expression&& right,
        region_type region) noexcept:
    quantified_comparison_predicate {
            clone_unique(std::move(left)),
            operator_kind,
            quantifier,
            clone_unique(std::move(right)),
            region,
    }
{}

quantified_comparison_predicate::quantified_comparison_predicate(::takatori::util::clone_tag_t, quantified_comparison_predicate const& other) :
    quantified_comparison_predicate {
            clone_unique(other.left_),
            other.operator_kind_,
            other.quantifier_,
            clone_unique(other.right_),
            other.region(),
    }
{}

quantified_comparison_predicate::quantified_comparison_predicate(::takatori::util::clone_tag_t, quantified_comparison_predicate&& other) :
    quantified_comparison_predicate {
            clone_unique(std::move(other.left_)),
            other.operator_kind_,
            other.quantifier_,
            clone_unique(std::move(other.right_)),
            other.region(),
    }
{}

quantified_comparison_predicate* quantified_comparison_predicate::clone() const& {
    return new quantified_comparison_predicate(::takatori::util::clone_tag, *this); // NOLINT
}

quantified_comparison_predicate* quantified_comparison_predicate::clone() && {
    return new quantified_comparison_predicate(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

expression::node_kind_type quantified_comparison_predicate::node_kind() const noexcept {
    return tag;
}

quantified_comparison_predicate::operator_kind_type& quantified_comparison_predicate::operator_kind() noexcept {
    return operator_kind_;
}

quantified_comparison_predicate::operator_kind_type const& quantified_comparison_predicate::operator_kind() const noexcept {
    return operator_kind_;
}

quantified_comparison_predicate::quantifier_type& quantified_comparison_predicate::quantifier() noexcept {
    return quantifier_;
}

quantified_comparison_predicate::quantifier_type const& quantified_comparison_predicate::quantifier() const noexcept {
    return quantifier_;
}

expression::operand_type& quantified_comparison_predicate::left() noexcept {
    return left_;
}

expression::operand_type const& quantified_comparison_predicate::left() const noexcept {
    return left_;
}

std::unique_ptr<query::expression>& quantified_comparison_predicate::right() noexcept {
    return right_;
}

std::unique_ptr<query::expression> const& quantified_comparison_predicate::right() const noexcept {
    return right_;
}

bool operator==(quantified_comparison_predicate const& a, quantified_comparison_predicate const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return false;
    }
    return eq(a.operator_kind_, b.operator_kind_)
            && eq(a.left_, b.left_)
            && eq(a.right_, b.right_)
            && eq(a.quantifier_, b.quantifier_);
}

bool operator!=(quantified_comparison_predicate const& a, quantified_comparison_predicate const& b) noexcept {
    return !(a == b);
}

bool quantified_comparison_predicate::equals(expression const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void quantified_comparison_predicate::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "left"sv, left_);
    property(acceptor, "operator_kind"sv, operator_kind_);
    property(acceptor, "quantifier"sv, quantifier_);
    property(acceptor, "right"sv, right_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, quantified_comparison_predicate const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::scalar
