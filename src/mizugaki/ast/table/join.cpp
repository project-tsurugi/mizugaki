#include <mizugaki/ast/table/join.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::table {

using ::takatori::util::clone_unique;
using ::takatori::util::rvalue_ptr;

join::join(
        std::unique_ptr<table::expression> left,
        operator_kind_type operator_kind,
        std::unique_ptr<table::expression> right,
        std::unique_ptr<join_specification> specification,
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

join::join(::takatori::util::clone_tag_t, join const& other) :
    join {
            clone_unique(other.left_),
            other.operator_kind_,
            clone_unique(other.right_),
            clone_unique(other.specification_),
            other.region(),
    }
{}

join::join(::takatori::util::clone_tag_t, join&& other) :
    join {
            clone_unique(std::move(other.left_)),
            other.operator_kind_,
            clone_unique(std::move(other.right_)),
            clone_unique(std::move(other.specification_)),
            other.region(),
    }
{}

join* join::clone() const& {
    return new join(::takatori::util::clone_tag, *this); // NOLINT
}

join* join::clone() && {
    return new join(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
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

std::unique_ptr<table::expression>& join::left() noexcept {
    return left_;
}

std::unique_ptr<table::expression> const& join::left() const noexcept {
    return left_;
}

std::unique_ptr<table::expression>& join::right() noexcept {
    return right_;
}

std::unique_ptr<table::expression> const& join::right() const noexcept {
    return right_;
}

std::unique_ptr<join_specification>& join::specification() noexcept {
    return specification_;
}

std::unique_ptr<join_specification> const& join::specification() const noexcept {
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
