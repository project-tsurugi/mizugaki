#include <mizugaki/ast/statement/identity_constraint.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

#include "utils.h"

namespace mizugaki::ast::statement {

using ::takatori::util::clone_unique;

identity_constraint::identity_constraint(
        generation_type generation,
        std::unique_ptr<scalar::expression> initial_value,
        std::unique_ptr<scalar::expression> increment_value,
        std::unique_ptr<scalar::expression> min_value,
        std::unique_ptr<scalar::expression> max_value,
        std::optional<bool_type> cycle,
        region_type region) noexcept :
    super { region },
    generation_ { generation },
    initial_value_ { std::move(initial_value) },
    increment_value_ { std::move(increment_value) },
    min_value_ { std::move(min_value) },
    max_value_ { std::move(max_value) },
    cycle_ { cycle }
{}

identity_constraint::identity_constraint(::takatori::util::clone_tag_t, identity_constraint const& other) :
    identity_constraint {
            other.generation_,
            clone_unique(other.initial_value_),
            clone_unique(other.increment_value_),
            clone_unique(other.min_value_),
            clone_unique(other.max_value_),
            other.cycle_,
            other.region(),
    }
{}

identity_constraint::identity_constraint(::takatori::util::clone_tag_t, identity_constraint&& other) :
        identity_constraint {
            other.generation_,
            std::move(other.initial_value_),
            std::move(other.increment_value_),
            std::move(other.min_value_),
            std::move(other.max_value_),
            other.cycle_,
            other.region(),
        }
{}

identity_constraint* identity_constraint::clone() const& {
    return new identity_constraint(::takatori::util::clone_tag, *this); // NOLINT
}

identity_constraint* identity_constraint::clone() && {
    return new identity_constraint(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

constraint::node_kind_type identity_constraint::node_kind() const noexcept {
    return tag;
}

identity_constraint::generation_type& identity_constraint::generation() noexcept {
    return generation_;
}

identity_constraint::generation_type const& identity_constraint::generation() const noexcept {
    return generation_;
}

std::unique_ptr<scalar::expression>& identity_constraint::initial_value() noexcept {
    return initial_value_;
}

std::unique_ptr<scalar::expression> const& identity_constraint::initial_value() const noexcept {
    return initial_value_;
}

std::unique_ptr<scalar::expression>& identity_constraint::increment_value() noexcept {
    return increment_value_;
}

std::unique_ptr<scalar::expression> const& identity_constraint::increment_value() const noexcept {
    return increment_value_;
}

std::unique_ptr<scalar::expression>& identity_constraint::min_value() noexcept {
    return min_value_;
}

std::unique_ptr<scalar::expression> const& identity_constraint::min_value() const noexcept {
    return min_value_;
}

std::unique_ptr<scalar::expression>& identity_constraint::max_value() noexcept {
    return max_value_;
}

std::unique_ptr<scalar::expression> const& identity_constraint::max_value() const noexcept {
    return max_value_;
}

std::optional<identity_constraint::bool_type>& identity_constraint::cycle() noexcept {
    return cycle_;
}

std::optional<identity_constraint::bool_type> const& identity_constraint::cycle() const noexcept {
    return cycle_;
}


bool operator==(identity_constraint const& a, identity_constraint const& b) noexcept {
    return eq(a.generation_, b.generation_)
        && eq(a.initial_value_, b.initial_value_)
        && eq(a.increment_value_, b.increment_value_)
        && eq(a.min_value_, b.min_value_)
        && eq(a.max_value_, b.max_value_)
        && eq(a.cycle_, b.cycle_);
}

bool operator!=(identity_constraint const& a, identity_constraint const& b) noexcept {
    return !(a == b);
}

bool identity_constraint::equals(constraint const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void identity_constraint::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "generation"sv, generation_);
    property(acceptor, "initial_value"sv, initial_value_);
    property(acceptor, "increment_value"sv, increment_value_);
    property(acceptor, "min_value"sv, min_value_);
    property(acceptor, "max_value"sv, max_value_);
    property(acceptor, "cycle"sv, cycle_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, identity_constraint const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::statement
