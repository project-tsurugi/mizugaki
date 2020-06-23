#include <mizugaki/ast/table/join.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::table {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::rvalue_ptr;
using ::takatori::util::unique_object_ptr;

join::join(
        unique_object_ptr<table::expression> left,
        operator_kind_type operator_kind,
        unique_object_ptr<table::expression> right,
        ::takatori::util::unique_object_ptr<join_specification> specification,
        element::region_type region) noexcept :
    super { region },
    left_ { std::move(left) },
    operator_kind_ { operator_kind },
    right_ { std::move(right) },
    specification_ { std::move(specification) }
{}

join::join(
        expression&& left,
        operator_kind_type operator_kind,
        expression&& right,
        rvalue_ptr<join_specification> specification,
        region_type region) :
    join {
            clone_unique(std::move(left)),
            operator_kind,
            clone_unique(std::move(right)),
            clone_unique(specification),
            region,
    }
{}

join::join(join const& other, object_creator creator) :
    join {
            clone_unique(other.left_, creator),
            other.operator_kind_,
            clone_unique(other.right_, creator),
            clone_unique(other.specification_, creator),
            other.region(),
    }
{}

join::join(join&& other, object_creator creator) :
    join {
            clone_unique(std::move(other.left_), creator),
            other.operator_kind_,
            clone_unique(std::move(other.right_), creator),
            clone_unique(std::move(other.specification_), creator),
            other.region(),
    }
{}

join* join::clone(object_creator creator) const& {
    return creator.create_object<join>(*this, creator);
}

join* join::clone(object_creator creator) && {
    return creator.create_object<join>(std::move(*this), creator);
}

expression::node_kind_type join::node_kind() const noexcept {
    return tag;
}

join::operator_kind_type& join::operator_kind() noexcept {
    return operator_kind_;
}

join::operator_kind_type const& join::operator_kind() const noexcept {
    return operator_kind_;
}

unique_object_ptr<table::expression>& join::left() noexcept {
    return left_;
}

unique_object_ptr<table::expression> const& join::left() const noexcept {
    return left_;
}

unique_object_ptr<table::expression>& join::right() noexcept {
    return right_;
}

unique_object_ptr<table::expression> const& join::right() const noexcept {
    return right_;
}

unique_object_ptr<join_specification>& join::specification() noexcept {
    return specification_;
}

unique_object_ptr<join_specification> const& join::specification() const noexcept {
    return specification_;
}

bool operator==(join const& a, join const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return true;
    }
    return eq(a.operator_kind_, b.operator_kind_)
            && eq(a.left_, b.left_)
            && eq(a.right_, b.right_)
            && eq(a.specification_, b.specification_);
}

bool operator!=(join const& a, join const& b) noexcept {
    return !(a == b);
}

bool join::equals(expression const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void join::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "left"sv, left_);
    property(acceptor, "operator_kind"sv, operator_kind_);
    property(acceptor, "right"sv, right_);
    property(acceptor, "specification"sv, specification_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, join const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::table
