#include <mizugaki/ast/table/join_condition.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::table {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

join_condition::join_condition(
        unique_object_ptr<scalar::expression> expression,
        region_type region) noexcept :
    super { region },
    expression_ { std::move(expression) }
{}

join_condition::join_condition(join_condition const& other, object_creator creator) :
    join_condition {
            clone_unique(other.expression_, creator),
            other.region(),
    }
{}

join_condition::join_condition(join_condition&& other, object_creator creator) :
    join_condition {
            clone_unique(std::move(other.expression_), creator),
            other.region(),
    }
{}

join_condition* join_condition::clone(object_creator creator) const& {
    return creator.create_object<join_condition>(*this, creator);
}

join_condition* join_condition::clone(object_creator creator)&& {
    return creator.create_object<join_condition>(std::move(*this), creator);
}

join_specification::node_kind_type join_condition::node_kind() const noexcept {
    return tag;
}

unique_object_ptr<scalar::expression>& join_condition::expression() noexcept {
    return expression_;
}

unique_object_ptr<scalar::expression> const& join_condition::expression() const noexcept {
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

} // namespace mizugaki::ast::table
