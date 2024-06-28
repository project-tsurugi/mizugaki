#include <mizugaki/ast/statement/expression_constraint.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

#include "utils.h"

namespace mizugaki::ast::statement {

using ::takatori::util::clone_unique;

expression_constraint::expression_constraint(
        constraint_kind_type constraint_kind,
        std::unique_ptr<scalar::expression> expression,
        region_type region) :
    super { region },
    constraint_kind_ { constraint_kind },
    expression_ { std::move(expression) }
{
    utils::validate_kind(tags, *constraint_kind);
}

expression_constraint::expression_constraint(
        constraint_kind_type constraint_kind,
        scalar::expression&& expression,
        element::region_type region) :
    expression_constraint {
            constraint_kind,
            clone_unique(std::move(expression)),
            region,
    }
{}

expression_constraint::expression_constraint(::takatori::util::clone_tag_t, expression_constraint const& other) :
    expression_constraint {
            other.constraint_kind_,
            clone_unique(other.expression_),
            other.region(),
    }
{}

expression_constraint::expression_constraint(::takatori::util::clone_tag_t, expression_constraint&& other) :
        expression_constraint {
                other.constraint_kind_,
                std::move(other.expression_),
                other.region(),
        }
{}

expression_constraint* expression_constraint::clone() const& {
    return new expression_constraint(::takatori::util::clone_tag, *this); // NOLINT
}

expression_constraint* expression_constraint::clone() && {
    return new expression_constraint(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

constraint::node_kind_type expression_constraint::node_kind() const noexcept {
    return *constraint_kind_;
}

expression_constraint::constraint_kind_type& expression_constraint::constraint_kind() noexcept {
    return constraint_kind_;
}

expression_constraint::constraint_kind_type const& expression_constraint::constraint_kind() const noexcept {
    return constraint_kind_;
}

std::unique_ptr<scalar::expression>& expression_constraint::expression() noexcept {
    return expression_;
}

std::unique_ptr<scalar::expression> const& expression_constraint::expression() const noexcept {
    return expression_;
}

bool operator==(expression_constraint const& a, expression_constraint const& b) noexcept {
    return eq(a.constraint_kind_, b.constraint_kind_)
        && eq(a.expression_, b.expression_);
}

bool operator!=(expression_constraint const& a, expression_constraint const& b) noexcept {
    return !(a == b);
}

bool expression_constraint::equals(constraint const& other) const noexcept {
    return tags.contains(other.node_kind())
            && *this == unsafe_downcast<expression_constraint>(other);
}

void expression_constraint::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "expression"sv, expression_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, expression_constraint const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::statement
