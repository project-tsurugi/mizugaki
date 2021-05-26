#include <mizugaki/ast/scalar/table_predicate.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::scalar {

using ::takatori::util::clone_unique;

table_predicate::table_predicate(
        operator_kind_type operator_kind,
        std::unique_ptr<query::expression> operand,
        region_type region) noexcept :
    super { region },
    operator_kind_ { operator_kind },
    operand_ { std::move(operand) }
{}

table_predicate::table_predicate(
        operator_kind_type operator_kind,
        query::expression&& operand,
        region_type region) :
    table_predicate {
        operator_kind,
        clone_unique(std::move(operand)),
        region,
    }
{}

table_predicate::table_predicate(::takatori::util::clone_tag_t, table_predicate const& other) :
    table_predicate {
            other.operator_kind_,
            clone_unique(other.operand_),
            other.region(),
    }
{}

table_predicate::table_predicate(::takatori::util::clone_tag_t, table_predicate&& other) :
    table_predicate {
            other.operator_kind_,
            clone_unique(std::move(other.operand_)),
            other.region(),
    }
{}

table_predicate* table_predicate::clone() const& {
    return new table_predicate(::takatori::util::clone_tag, *this); // NOLINT
}

table_predicate* table_predicate::clone() && {
    return new table_predicate(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

expression::node_kind_type table_predicate::node_kind() const noexcept {
    return tag;
}

table_predicate::operator_kind_type& table_predicate::operator_kind() noexcept {
    return operator_kind_;
}

table_predicate::operator_kind_type const& table_predicate::operator_kind() const noexcept {
    return operator_kind_;
}

std::unique_ptr<query::expression>& table_predicate::operand() noexcept {
    return operand_;
}

std::unique_ptr<query::expression> const& table_predicate::operand() const noexcept {
    return operand_;
}

bool operator==(table_predicate const& a, table_predicate const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return false;
    }
    return eq(a.operator_kind_, b.operator_kind_)
            && eq(a.operand_, b.operand_);
}

bool operator!=(table_predicate const& a, table_predicate const& b) noexcept {
    return !(a == b);
}

bool table_predicate::equals(expression const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void table_predicate::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "operator_kind"sv, operator_kind_);
    property(acceptor, "operand"sv, operand_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, table_predicate const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::scalar
