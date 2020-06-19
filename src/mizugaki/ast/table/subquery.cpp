#include <mizugaki/ast/table/subquery.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::table {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

subquery::subquery(
        bool_type is_lateral,
        unique_object_ptr<query::expression> expression,
        correlation_type correlation,
        region_type region) noexcept :
    super { region },
    is_lateral_ { is_lateral },
    expression_ { std::move(expression) },
    correlation_ { std::move(correlation) }
{}

subquery::subquery(subquery const& other, object_creator creator) :
    subquery {
            other.is_lateral_,
            clone_unique(other.expression_, creator),
            decltype(correlation_) { other.correlation_, creator },
            other.region(),
    }
{}

subquery::subquery(subquery&& other, object_creator creator) :
    subquery {
            other.is_lateral_,
            clone_unique(std::move(other.expression_), creator),
            decltype(correlation_) { std::move(other.correlation_), creator },
            other.region(),
    }
{}

subquery* subquery::clone(object_creator creator) const& {
    return creator.create_object<subquery>(*this, creator);
}

subquery* subquery::clone(object_creator creator) && {
    return creator.create_object<subquery>(std::move(*this), creator);
}

expression::node_kind_type subquery::node_kind() const noexcept {
    return tag;
}

unique_object_ptr<query::expression>& subquery::expression() noexcept {
    return expression_;
}

unique_object_ptr<query::expression> const& subquery::expression() const noexcept {
    return expression_;
}

subquery::correlation_type& subquery::correlation() noexcept {
    return correlation_;
}

subquery::correlation_type const& subquery::correlation() const noexcept {
    return correlation_;
}

subquery::bool_type& subquery::is_lateral() noexcept {
    return is_lateral_;
}

subquery::bool_type const& subquery::is_lateral() const noexcept {
    return is_lateral_;
}

bool operator==(subquery const& a, subquery const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return true;
    }
    return eq(a.expression_, b.expression_)
            && eq(a.correlation_, b.correlation_)
            && eq(a.is_lateral_, b.is_lateral_);
}

bool operator!=(subquery const& a, subquery const& b) noexcept {
    return !(a == b);
}

bool subquery::equals(table::expression const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void subquery::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "is_lateral"sv, is_lateral_);
    property(acceptor, "expression"sv, expression_);
    property(acceptor, "correlation"sv, correlation_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, subquery const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::table
