#include <mizugaki/ast/statement/drop_partition_action.h>

#include <mizugaki/ast/common/serializers.h>
#include <mizugaki/ast/common/vector.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::statement {

using common::clone_vector;

drop_partition_action::drop_partition_action(
        std::vector<partition_criteria> target,
        region_type region) noexcept:
    super { region },
    target_ { std::move(target) }
{}

drop_partition_action::drop_partition_action(
        std::initializer_list<partition_criteria> target,
        region_type region):
    drop_partition_action {
        std::vector<partition_criteria> { target },
        region,
    }
{}

drop_partition_action::drop_partition_action(
        ::takatori::util::clone_tag_t,
        drop_partition_action const& other):
    drop_partition_action {
        clone_vector(other.target_),
        other.region(),
    }
{}

drop_partition_action::drop_partition_action(
        ::takatori::util::clone_tag_t,
        drop_partition_action&& other):
    drop_partition_action {
        clone_vector(std::move(other.target_)),
        other.region(),
    }
{}

drop_partition_action* drop_partition_action::clone() const & {
    return new drop_partition_action(::takatori::util::clone_tag, *this); // NOLINT
}

drop_partition_action* drop_partition_action::clone() && {
    return new drop_partition_action(::takatori::util::clone_tag, std::move(*this)); // NOLINT
}

alter_table_action::node_kind_type drop_partition_action::node_kind() const noexcept {
    return tag;
}

std::vector<partition_criteria>& drop_partition_action::target() noexcept {
    return target_;
}

std::vector<partition_criteria> const& drop_partition_action::target() const noexcept {
    return target_;
}

bool operator==(drop_partition_action const& a, drop_partition_action const& b) noexcept {
    return eq(a.target_, b.target_);
}

bool operator!=(drop_partition_action const& a, drop_partition_action const& b) noexcept {
    return !(a == b);
}

bool drop_partition_action::equals(alter_table_action const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<drop_partition_action>(other);
}

void drop_partition_action::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "target"sv, target_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, drop_partition_action const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::statement
