#include <mizugaki/ast/statement/simple_constraint.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

#include "utils.h"

namespace mizugaki::ast::statement {

simple_constraint::simple_constraint(
        constraint_kind_type constraint_kind,
        region_type region) :
    super { region },
    constraint_kind_ { constraint_kind }
{
    utils::validate_kind(tags, *constraint_kind);
}

simple_constraint::simple_constraint(::takatori::util::clone_tag_t, simple_constraint const& other) :
    simple_constraint {
            other.constraint_kind_,
            other.region(),
    }
{}

simple_constraint::simple_constraint(::takatori::util::clone_tag_t, simple_constraint&& other) :
        simple_constraint {
                other.constraint_kind_,
                other.region(),
        }
{}

simple_constraint* simple_constraint::clone() const& {
    return new simple_constraint(::takatori::util::clone_tag, *this); // NOLINT
}

simple_constraint* simple_constraint::clone() && {
    return new simple_constraint(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

constraint::node_kind_type simple_constraint::node_kind() const noexcept {
    return *constraint_kind_;
}

simple_constraint::constraint_kind_type& simple_constraint::constraint_kind() noexcept {
    return constraint_kind_;
}

simple_constraint::constraint_kind_type const& simple_constraint::constraint_kind() const noexcept {
    return constraint_kind_;
}

bool operator==(simple_constraint const& a, simple_constraint const& b) noexcept {
    return eq(a.constraint_kind_, b.constraint_kind_);
}

bool operator!=(simple_constraint const& a, simple_constraint const& b) noexcept {
    return !(a == b);
}

bool simple_constraint::equals(constraint const& other) const noexcept {
    return tags.contains(other.node_kind())
            && *this == unsafe_downcast<simple_constraint>(other);
}

void simple_constraint::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, simple_constraint const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::statement
