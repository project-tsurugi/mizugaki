#include <mizugaki/ast/scalar/subquery.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::scalar {

using ::takatori::util::clone_unique;

subquery::subquery(
        std::unique_ptr<query::expression> expression,
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

subquery::subquery(::takatori::util::clone_tag_t, subquery const& other) :
    subquery {
            clone_unique(other.expression_),
            other.region(),
    }
{}

subquery::subquery(::takatori::util::clone_tag_t, subquery&& other) :
    subquery {
            clone_unique(std::move(other.expression_)),
            other.region(),
    }
{}

subquery* subquery::clone() const& {
    return new subquery(::takatori::util::clone_tag, *this); // NOLINT
}

subquery* subquery::clone() && {
    return new subquery(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

expression::node_kind_type subquery::node_kind() const noexcept {
    return tag;
}

std::unique_ptr<query::expression>& subquery::expression() noexcept {
    return expression_;
}

std::unique_ptr<query::expression> const& subquery::expression() const noexcept {
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
