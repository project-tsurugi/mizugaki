#include <mizugaki/ast/statement/revoke_privilege_statement.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::statement {

revoke_privilege_statement::revoke_privilege_statement(
        std::vector<privilege_action> actions,
        std::vector<privilege_object> objects,
        std::vector<privilege_user> users,
        region_type region) noexcept :
    super { region },
    actions_ { std::move(actions) },
    objects_ { std::move(objects) },
    users_ { std::move(users) }
{}

revoke_privilege_statement::revoke_privilege_statement(::takatori::util::clone_tag_t, revoke_privilege_statement const& other) :
    revoke_privilege_statement {
            other.actions_,
            other.objects_,
            other.users_,
            other.region(),
    }
{}

revoke_privilege_statement::revoke_privilege_statement(::takatori::util::clone_tag_t, revoke_privilege_statement&& other) :
    revoke_privilege_statement {
            std::move(other.actions_),
            std::move(other.objects_),
            std::move(other.users_),
            other.region(),
    }
{}

revoke_privilege_statement* revoke_privilege_statement::clone() const& {
    return new revoke_privilege_statement(::takatori::util::clone_tag, *this); // NOLINT
}

revoke_privilege_statement* revoke_privilege_statement::clone() && {
    return new revoke_privilege_statement(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

statement::node_kind_type revoke_privilege_statement::node_kind() const noexcept {
    return tag;
}

std::vector<privilege_action>& revoke_privilege_statement::actions() noexcept {
    return actions_;
}

std::vector<privilege_action> const& revoke_privilege_statement::actions() const noexcept {
    return actions_;
}

std::vector<privilege_object>& revoke_privilege_statement::objects() noexcept {
    return objects_;
}

std::vector<privilege_object> const& revoke_privilege_statement::objects() const noexcept {
    return objects_;
}

std::vector<privilege_user>& revoke_privilege_statement::users() noexcept {
    return users_;
}

std::vector<privilege_user> const& revoke_privilege_statement::users() const noexcept {
    return users_;
}

bool operator==(revoke_privilege_statement const& a, revoke_privilege_statement const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return true;
    }
    return eq(a.actions_, b.actions_)
        && eq(a.objects_, b.objects_)
        && eq(a.users_, b.users_);
}

bool operator!=(revoke_privilege_statement const& a, revoke_privilege_statement const& b) noexcept {
    return !(a == b);
}

bool revoke_privilege_statement::equals(statement const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void revoke_privilege_statement::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "actions"sv, actions_);
    property(acceptor, "objects"sv, objects_);
    property(acceptor, "users"sv, users_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, revoke_privilege_statement const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::statement
