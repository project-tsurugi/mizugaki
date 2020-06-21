#include <mizugaki/ast/scalar/comparison_predicate.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::scalar {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

comparison_predicate::comparison_predicate(
        operand_type left,
        operator_kind_type operator_kind,
        operand_type right,
        region_type region) noexcept:
    super { region },
    left_ { std::move(left) },
    operator_kind_ { operator_kind },
    right_ { std::move(right) }
{}

comparison_predicate::comparison_predicate(
        expression&& left,
        comparison_operator operator_kind,
        expression&& right,
        region_type region) noexcept :
    comparison_predicate {
            clone_unique(std::move(left)),
            operator_kind,
            clone_unique(std::move(right)),
            region,
    }
{}

comparison_predicate::comparison_predicate(comparison_predicate const& other, object_creator creator) :
    comparison_predicate {
            clone_unique(other.left_, creator),
            other.operator_kind_,
            clone_unique(other.right_, creator),
            other.region(),
    }
{}

comparison_predicate::comparison_predicate(comparison_predicate&& other, object_creator creator) :
    comparison_predicate {
            clone_unique(std::move(other.left_), creator),
            other.operator_kind_,
            clone_unique(std::move(other.right_), creator),
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

bool operator==(comparison_predicate const& a, comparison_predicate const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return false;
    }
    return eq(a.operator_kind_, b.operator_kind_)
            && eq(a.left_, b.left_)
            && eq(a.right_, b.right_);
}

bool operator!=(comparison_predicate const& a, comparison_predicate const& b) noexcept {
    return !(a == b);
}

bool comparison_predicate::equals(expression const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void comparison_predicate::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "left"sv, left_);
    property(acceptor, "operator_kind"sv, operator_kind_);
    property(acceptor, "right"sv, right_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, comparison_predicate const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::scalar
