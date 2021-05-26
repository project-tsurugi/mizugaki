#include <mizugaki/ast/table/join_condition.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::table {

using ::takatori::util::clone_unique;

join_condition::join_condition(
        std::unique_ptr<scalar::expression> expression,
        region_type region) noexcept :
    super { region },
    expression_ { std::move(expression) }
{}

join_condition::join_condition(
        scalar::expression&& expression,
        region_type region) :
    join_condition {
            clone_unique(std::move(expression)),
            region,
    }
{}

join_condition::join_condition(::takatori::util::clone_tag_t, join_condition const& other) :
    join_condition {
            clone_unique(other.expression_),
            other.region(),
    }
{}

join_condition::join_condition(::takatori::util::clone_tag_t, join_condition&& other) :
    join_condition {
            clone_unique(std::move(other.expression_)),
            other.region(),
    }
{}

join_condition* join_condition::clone() const& {
    return new join_condition(::takatori::util::clone_tag, *this); // NOLINT
}

join_condition* join_condition::clone()&& {
    return new join_condition(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

join_specification::node_kind_type join_condition::node_kind() const noexcept {
    return tag;
}

std::unique_ptr<scalar::expression>& join_condition::expression() noexcept {
    return expression_;
}

std::unique_ptr<scalar::expression> const& join_condition::expression() const noexcept {
    return expression_;
}

bool operator==(join_condition const& a, join_condition const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return false;
    }
    return eq(a.expression_, b.expression_);
}

bool operator!=(join_condition const& a, join_condition const& b) noexcept {
    return !(a == b);
}

bool join_condition::equals(join_specification const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void join_condition::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "expression"sv, expression_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, join_condition const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::table
