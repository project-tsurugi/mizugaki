#include <mizugaki/ast/statement/alter_table_statement.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::statement {

using ::takatori::util::clone_unique;

alter_table_statement::alter_table_statement(
        bool_type if_exists,
        std::unique_ptr<name::name> name,
        std::unique_ptr<alter_table_action> action,
        region_type region) noexcept:
    super { region },
    if_exists_ { if_exists },
    name_ { std::move(name) },
    action_ { std::move(action) }
{}

alter_table_statement::alter_table_statement(
        name::name&& name,
        alter_table_action&& action,
        bool_type if_exists,
        region_type region):
    alter_table_statement {
            if_exists,
            clone_unique(std::move(name)),
            clone_unique(std::move(action)),
            region,
    }
{}

alter_table_statement::alter_table_statement(takatori::util::clone_tag_t, alter_table_statement const& other):
    alter_table_statement {
            other.if_exists_,
            clone_unique(other.name_),
            clone_unique(other.action_),
            other.region(),
    }
{}

alter_table_statement::alter_table_statement(takatori::util::clone_tag_t, alter_table_statement&& other):
    alter_table_statement {
            other.if_exists_,
            std::move(other.name_),
            std::move(other.action_),
            other.region(),
    }
{}

alter_table_statement* alter_table_statement::clone() const & {
    return new alter_table_statement(::takatori::util::clone_tag, *this); // NOLINT
}

alter_table_statement* alter_table_statement::clone() && {
    return new alter_table_statement(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

statement::node_kind_type alter_table_statement::node_kind() const noexcept {
    return node_kind_type::alter_table_statement;
}

alter_table_statement::bool_type& alter_table_statement::if_exists() noexcept {
    return if_exists_;
}

alter_table_statement::bool_type alter_table_statement::if_exists() const noexcept {
    return if_exists_;
}

std::unique_ptr<name::name>& alter_table_statement::name() noexcept {
    return name_;
}

std::unique_ptr<name::name> const& alter_table_statement::name() const noexcept {
    return name_;
}

std::unique_ptr<alter_table_action>& alter_table_statement::action() noexcept {
    return action_;
}

std::unique_ptr<alter_table_action> const& alter_table_statement::action() const noexcept {
    return action_;
}

bool operator==(alter_table_statement const& a, alter_table_statement const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return true;
    }
    return eq(a.if_exists_, b.if_exists_)
        && eq(a.name_, b.name_)
        && eq(a.action_, b.action_);
}

bool operator!=(alter_table_statement const& a, alter_table_statement const& b) noexcept {
    return !(a == b);
}

bool alter_table_statement::equals(statement const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void alter_table_statement::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "if_exists"sv, if_exists_);
    property(acceptor, "name"sv, name_);
    property(acceptor, "action"sv, action_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, alter_table_statement const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::statement
