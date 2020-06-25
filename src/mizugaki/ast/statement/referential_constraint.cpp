#include <mizugaki/ast/statement/referential_constraint.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>
#include <mizugaki/ast/common/vector.h>

#include "utils.h"

namespace mizugaki::ast::statement {

using ::takatori::util::clone_unique;

using common::clone_vector;

referential_constraint::referential_constraint(
        std::vector<std::unique_ptr<name::simple>> columns,
        std::unique_ptr<name::name> target,
        std::vector<std::unique_ptr<name::simple>> target_columns,
        std::optional<action_type> on_update,
        std::optional<action_type> on_delete,
        region_type region) noexcept :
    super { region },
    columns_ { std::move(columns) },
    target_ { std::move(target) },
    target_columns_ { std::move(target_columns) },
    on_update_ { on_update },
    on_delete_ { on_delete }
{}

referential_constraint::referential_constraint(
        common::rvalue_list<name::simple> columns,
        name::name&& target,
        common::rvalue_list<name::simple> target_columns,
        std::optional<action_type> on_update,
        std::optional<action_type> on_delete,
        region_type region) :
    referential_constraint {
            common::to_vector(columns),
            clone_unique(std::move(target)),
            common::to_vector(target_columns),
            on_update,
            on_delete,
            region,
    }
{}

referential_constraint::referential_constraint(::takatori::util::clone_tag_t, referential_constraint const& other) :
    referential_constraint {
            clone_vector(other.columns_),
            clone_unique(other.target_),
            clone_vector(other.target_columns_),
            other.on_update_,
            other.on_delete_,
            other.region(),
    }
{}

referential_constraint::referential_constraint(::takatori::util::clone_tag_t, referential_constraint&& other) :
        referential_constraint {
            std::move(other.columns_),
            std::move(other.target_),
            std::move(other.target_columns_),
            other.on_update_,
            other.on_delete_,
            other.region(),
        }
{}

referential_constraint* referential_constraint::clone() const& {
    return new referential_constraint(::takatori::util::clone_tag, *this); // NOLINT
}

referential_constraint* referential_constraint::clone() && {
    return new referential_constraint(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

constraint::node_kind_type referential_constraint::node_kind() const noexcept {
    return tag;
}

std::vector<std::unique_ptr<name::simple>>& referential_constraint::columns() noexcept {
    return columns_;
}

std::vector<std::unique_ptr<name::simple>> const& referential_constraint::columns() const noexcept {
    return columns_;
}

std::unique_ptr<name::name>& referential_constraint::target() noexcept {
    return target_;
}

std::unique_ptr<name::name> const& referential_constraint::target() const noexcept {
    return target_;
}

std::vector<std::unique_ptr<name::simple>>& referential_constraint::target_columns() noexcept {
    return target_columns_;
}

std::vector<std::unique_ptr<name::simple>> const& referential_constraint::target_columns() const noexcept {
    return target_columns_;
}

std::optional<referential_constraint::action_type>& referential_constraint::on_update() noexcept {
    return on_update_;
}

std::optional<referential_constraint::action_type> const& referential_constraint::on_update() const noexcept {
    return on_update_;
}

std::optional<referential_constraint::action_type>& referential_constraint::on_delete() noexcept {
    return on_delete_;
}

std::optional<referential_constraint::action_type> const& referential_constraint::on_delete() const noexcept {
    return on_delete_;
}

bool operator==(referential_constraint const& a, referential_constraint const& b) noexcept {
    return eq(a.columns_, b.columns_)
        && eq(a.target_, b.target_)
        && eq(a.target_columns_, b.target_columns_)
        && eq(a.on_update_, b.on_update_)
        && eq(a.on_delete_, b.on_delete_);
}

bool operator!=(referential_constraint const& a, referential_constraint const& b) noexcept {
    return !(a == b);
}

bool referential_constraint::equals(constraint const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void referential_constraint::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "columns"sv, columns_);
    property(acceptor, "target"sv, target_);
    property(acceptor, "target_columns"sv, target_columns_);
    property(acceptor, "on_update"sv, on_update_);
    property(acceptor, "on_delete"sv, on_delete_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, referential_constraint const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::statement
