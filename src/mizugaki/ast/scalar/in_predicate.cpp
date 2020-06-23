#include <mizugaki/ast/scalar/in_predicate.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::scalar {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

in_predicate::in_predicate(
        operand_type left,
        bool_type is_not,
        unique_object_ptr<query::expression> right,
        region_type region) noexcept :
    super { region },
    left_ { std::move(left) },
    is_not_ { is_not },
    right_ { std::move(right) }
{}

in_predicate::in_predicate(
        expression&& left,
        query::expression&& right,
        bool_type is_not,
        region_type region) :
    in_predicate {
            clone_unique(std::move(left)),
            is_not,
            clone_unique(right),
            region,
    }
{}

in_predicate::in_predicate(in_predicate const& other, object_creator creator) :
    in_predicate {
            clone_unique(other.left_, creator),
            other.is_not_,
            clone_unique(other.right_, creator),
            other.region(),
    }
{}

in_predicate::in_predicate(in_predicate&& other, object_creator creator) :
    in_predicate {
            clone_unique(std::move(other.left_), creator),
            other.is_not_,
            clone_unique(std::move(other.right_), creator),
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

unique_object_ptr<query::expression>& in_predicate::right() noexcept {
    return right_;
}

unique_object_ptr<query::expression> const& in_predicate::right() const noexcept {
    return right_;
}

in_predicate::bool_type& in_predicate::is_not() noexcept {
    return is_not_;
}

in_predicate::bool_type const& in_predicate::is_not() const noexcept {
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
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void in_predicate::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "left"sv, left_);
    property(acceptor, "is_not"sv, is_not_);
    property(acceptor, "right"sv, right_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, in_predicate const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::scalar
