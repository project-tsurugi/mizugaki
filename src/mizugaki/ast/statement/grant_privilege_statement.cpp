#include <mizugaki/ast/statement/grant_privilege_statement.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::statement {

grant_privilege_statement::grant_privilege_statement(
        std::vector<privilege_action> actions,
        std::vector<privilege_object> objects,
        std::vector<privilege_user> users,
        region_type region) noexcept :
    super { region },
    actions_ { std::move(actions) },
    objects_ { std::move(objects) },
    users_ { std::move(users) }
{}

grant_privilege_statement::grant_privilege_statement(::takatori::util::clone_tag_t, grant_privilege_statement const& other) :
    grant_privilege_statement {
            other.actions_,
            other.objects_,
            other.users_,
            other.region(),
    }
{}

grant_privilege_statement::grant_privilege_statement(::takatori::util::clone_tag_t, grant_privilege_statement&& other) :
    grant_privilege_statement {
            std::move(other.actions_),
            std::move(other.objects_),
            std::move(other.users_),
            other.region(),
    }
{}

grant_privilege_statement* grant_privilege_statement::clone() const& {
    return new grant_privilege_statement(::takatori::util::clone_tag, *this); // NOLINT
}

grant_privilege_statement* grant_privilege_statement::clone() && {
    return new grant_privilege_statement(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

statement::node_kind_type grant_privilege_statement::node_kind() const noexcept {
    return tag;
}

std::vector<privilege_action>& grant_privilege_statement::actions() noexcept {
    return actions_;
}

std::vector<privilege_action> const& grant_privilege_statement::actions() const noexcept {
    return actions_;
}

std::vector<privilege_object>& grant_privilege_statement::objects() noexcept {
    return objects_;
}

std::vector<privilege_object> const& grant_privilege_statement::objects() const noexcept {
    return objects_;
}

std::vector<privilege_user>& grant_privilege_statement::users() noexcept {
    return users_;
}

std::vector<privilege_user> const& grant_privilege_statement::users() const noexcept {
    return users_;
}

bool operator==(grant_privilege_statement const& a, grant_privilege_statement const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return true;
    }
    return eq(a.actions_, b.actions_)
        && eq(a.objects_, b.objects_)
        && eq(a.users_, b.users_);
}

bool operator!=(grant_privilege_statement const& a, grant_privilege_statement const& b) noexcept {
    return !(a == b);
}

bool grant_privilege_statement::equals(statement const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void grant_privilege_statement::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "actions"sv, actions_);
    property(acceptor, "objects"sv, objects_);
    property(acceptor, "users"sv, users_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, grant_privilege_statement const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::statement
