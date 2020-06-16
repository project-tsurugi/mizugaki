#include <mizugaki/ast/statement/select_statement.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::statement {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

using common::clone_vector;

select_statement::select_statement(
        unique_object_ptr<query::expression> expression,
        common::vector<target_element> targets,
        region_type region) noexcept :
    super { region },
    expression_ { std::move(expression) },
    targets_ { std::move(targets) }
{}

select_statement::select_statement(select_statement const& other, object_creator creator) :
    select_statement {
            clone_unique(other.expression_, creator),
            clone_vector(other.targets_, creator),
            other.region(),
    }
{}

select_statement::select_statement(select_statement&& other, object_creator creator) :
    select_statement {
            clone_unique(std::move(other.expression_), creator),
            clone_vector(std::move(other.targets_), creator),
            other.region(),
    }
{}

select_statement* select_statement::clone(object_creator creator) const& {
    return creator.create_object<select_statement>(*this, creator);
}

select_statement* select_statement::clone(object_creator creator) && {
    return creator.create_object<select_statement>(std::move(*this), creator);
}

statement::node_kind_type select_statement::node_kind() const noexcept {
    return tag;
}

unique_object_ptr<query::expression>& select_statement::expression() noexcept {
    return expression_;
}

unique_object_ptr<query::expression> const& select_statement::expression() const noexcept {
    return expression_;
}

common::vector<target_element>& select_statement::targets() noexcept {
    return targets_;
}

common::vector<target_element> const& select_statement::targets() const noexcept {
    return targets_;
}

bool operator==(select_statement const& a, select_statement const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return true;
    }
    return eq(a.expression_, b.expression_)
            && eq(a.targets_, b.targets_);
}

bool operator!=(select_statement const& a, select_statement const& b) noexcept {
    return !(a == b);
}

bool select_statement::equals(statement const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<select_statement>(other);
}

} // namespace mizugaki::ast::statement
