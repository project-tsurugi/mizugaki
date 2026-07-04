#include <mizugaki/ast/statement/rename_column_action.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::statement {

using ::takatori::util::clone_unique;

rename_column_action::rename_column_action(
        bool_type if_exists,
        std::unique_ptr<name::simple> column_name,
        std::unique_ptr<name::simple> replacement,
        region_type region) noexcept:
    super { region },
    if_exists_ { if_exists },
    column_name_ { std::move(column_name) },
    replacement_ { std::move(replacement) }
{}

rename_column_action::rename_column_action(
        name::simple&& column_name,
        name::simple&& replacement,
        bool_type if_exists,
        region_type region):
    rename_column_action {
        if_exists,
        clone_unique(std::move(column_name)),
        clone_unique(std::move(replacement)),
        region,
    }
{}

rename_column_action::rename_column_action(
        ::takatori::util::clone_tag_t,
        rename_column_action const& other):
    rename_column_action {
        other.if_exists_,
        clone_unique(other.column_name_),
        clone_unique(other.replacement_),
        other.region(),
    }
{}

rename_column_action::rename_column_action(
        ::takatori::util::clone_tag_t,
        rename_column_action&& other):
    rename_column_action {
        other.if_exists_,
        clone_unique(std::move(other.column_name_)),
        clone_unique(std::move(other.replacement_)),
        other.region(),
    }
{}

rename_column_action* rename_column_action::clone() const & {
    return new rename_column_action(::takatori::util::clone_tag, *this); // NOLINT
}

rename_column_action* rename_column_action::clone() && {
    return new rename_column_action(::takatori::util::clone_tag, std::move(*this)); // NOLINT
}

alter_table_action::node_kind_type rename_column_action::node_kind() const noexcept {
    return node_kind_type::rename_column;
}

rename_column_action::bool_type& rename_column_action::if_exists() noexcept {
    return if_exists_;
}

rename_column_action::bool_type rename_column_action::if_exists() const noexcept {
    return if_exists_;
}

std::unique_ptr<name::simple>& rename_column_action::column_name() noexcept {
    return column_name_;
}

std::unique_ptr<name::simple> const& rename_column_action::column_name() const noexcept {
    return column_name_;
}

std::unique_ptr<name::simple>& rename_column_action::replacement() noexcept {
    return replacement_;
}

std::unique_ptr<name::simple> const& rename_column_action::replacement() const noexcept {
    return replacement_;
}

bool operator==(rename_column_action const& a, rename_column_action const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return true;
    }
    return eq(a.if_exists_, b.if_exists_)
        && eq(a.column_name_, b.column_name_)
        && eq(a.replacement_, b.replacement_);
}

bool operator!=(rename_column_action const& a, rename_column_action const& b) noexcept {
    return !(a == b);
}

bool rename_column_action::equals(alter_table_action const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<rename_column_action>(other);
}

void rename_column_action::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "if_exists"sv, if_exists_);
    property(acceptor, "column_name"sv, column_name_);
    property(acceptor, "replacement"sv, replacement_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, rename_column_action const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::statement
