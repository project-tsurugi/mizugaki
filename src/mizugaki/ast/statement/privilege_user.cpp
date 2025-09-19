#include <mizugaki/ast/statement/privilege_user.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::statement {

using ::takatori::util::clone_unique;

privilege_user::privilege_user(
        user_kind_type user_kind,
        std::unique_ptr<name::simple> authorization_identifier,
        region_type region) noexcept :
    element { region },
    user_kind_ { user_kind },
    authorization_identifier_ { std::move(authorization_identifier) }
{}

privilege_user::privilege_user(user_kind_type user_kind, region_type region):
    privilege_user {
            user_kind,
            nullptr,
            region,
    }
{
    if (user_kind == user_kind_type::identifier){
        throw std::invalid_argument {
                "user_kind must not be identifier when authorization_identifier is not specified"
        };
    }
}


privilege_user::privilege_user(name::simple&& authorization_identifier, region_type region) :
    privilege_user {
            user_kind_type::identifier,
            clone_unique(std::move(authorization_identifier)),
            region,
    }
{}


privilege_user::privilege_user(::takatori::util::clone_tag_t, privilege_user const& other) :
    privilege_user {
            other.user_kind(),
            clone_unique(*other.authorization_identifier_),
            other.region(),
    }
{}

privilege_user::privilege_user(::takatori::util::clone_tag_t, privilege_user&& other) :
    privilege_user {
            other.user_kind(),
            clone_unique(std::move(*other.authorization_identifier_)),
            other.region(),
    }
{}

privilege_user::user_kind_type& privilege_user::user_kind() noexcept {
    return user_kind_;
}

privilege_user::user_kind_type const& privilege_user::user_kind() const noexcept {
    return user_kind_;
}

std::unique_ptr<name::simple>& privilege_user::authorization_identifier() noexcept {
    return *authorization_identifier_;
}

std::unique_ptr<name::simple> const& privilege_user::authorization_identifier() const noexcept {
    return *authorization_identifier_;
}

bool operator==(privilege_user const& a, privilege_user const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return false;
    }
    if (!eq(a.user_kind_, b.user_kind_)) {
        return false;
    }
    if (a.user_kind_ == privilege_user::user_kind_type::identifier) {
        return eq(*a.authorization_identifier_, *b.authorization_identifier_);
    }
    return true;
}

bool operator!=(privilege_user const& a, privilege_user const& b) noexcept {
    return !(a == b);
}

::takatori::serializer::object_acceptor& operator<<(::takatori::serializer::object_acceptor& acceptor, privilege_user const& value) {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor);
    property(acceptor, "user_kind"sv, value.user_kind_);
    if (value.user_kind_ == privilege_user::user_kind_type::identifier) {
        property(acceptor, "authorization_identifier"sv, *value.authorization_identifier_);
    }
    region_property(acceptor, value);
    return acceptor;
}

std::ostream& operator<<(std::ostream& out, privilege_user const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::statement
