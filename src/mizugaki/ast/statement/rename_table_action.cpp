#include <mizugaki/ast/statement/rename_table_action.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::statement {

using ::takatori::util::clone_unique;

rename_table_action::rename_table_action(
        std::unique_ptr<name::simple> replacement,
        region_type region) noexcept:
    super { region },
    replacement_ { std::move(replacement) }
{}

rename_table_action::rename_table_action(name::simple&& replacement, region_type region):
    rename_table_action {
        clone_unique(std::move(replacement)),
        region,
    }
{}

rename_table_action::rename_table_action(takatori::util::clone_tag_t, rename_table_action const& other):
    rename_table_action {
        clone_unique(other.replacement_),
        other.region(),
    }
{}

rename_table_action::rename_table_action(takatori::util::clone_tag_t, rename_table_action&& other):
    rename_table_action {
        clone_unique(std::move(other.replacement_)),
        other.region(),
    }
{}

rename_table_action* rename_table_action::clone() const & {
    return new rename_table_action(::takatori::util::clone_tag, *this); // NOLINT
}

rename_table_action* rename_table_action::clone() && {
    return new rename_table_action(::takatori::util::clone_tag, std::move(*this)); // NOLINT
}

alter_table_action::node_kind_type rename_table_action::node_kind() const noexcept {
    return node_kind_type::rename_table;
}

std::unique_ptr<name::simple>& rename_table_action::replacement() noexcept {
    return replacement_;
}

std::unique_ptr<name::simple> const& rename_table_action::replacement() const noexcept {
    return replacement_;
}

bool operator==(rename_table_action const& a, rename_table_action const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return true;
    }
    return eq(a.replacement_, b.replacement_);
}

bool operator!=(rename_table_action const& a, rename_table_action const& b) noexcept {
    return !(a == b);
}

bool rename_table_action::equals(alter_table_action const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<rename_table_action>(other);
}

void rename_table_action::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "replacement"sv, replacement_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, rename_table_action const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::statement
