#include <mizugaki/ast/scalar/subquery.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::scalar {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

subquery::subquery(
        unique_object_ptr<query::expression> expression,
        region_type region) noexcept :
    super { region },
    expression_ { std::move(expression) }
{}

subquery::subquery(
        query::expression&& expression,
        region_type region) noexcept :
    subquery {
            clone_unique(std::move(expression)),
            region,
    }
{}

subquery::subquery(subquery const& other, object_creator creator) :
    subquery {
            clone_unique(other.expression_, creator),
            other.region(),
    }
{}

subquery::subquery(subquery&& other, object_creator creator) :
    subquery {
            clone_unique(std::move(other.expression_), creator),
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

bool operator==(subquery const& a, subquery const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return false;
    }
    return eq(a.expression_, b.expression_);
}

bool operator!=(subquery const& a, subquery const& b) noexcept {
    return !(a == b);
}

bool subquery::equals(scalar::expression const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void subquery::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "expression"sv, expression_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, subquery const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::scalar
