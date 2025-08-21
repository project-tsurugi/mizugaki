#include <mizugaki/ast/statement/privilege_user.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::statement {

using ::takatori::util::clone_unique;

privilege_user::privilege_user(std::unique_ptr<name::simple> authorization_identifier, region_type region) noexcept :
    element { region },
    authorization_identifier_ { std::move(authorization_identifier) }
{}

privilege_user::privilege_user(region_type region) noexcept :
    privilege_user {
            nullptr,
            region,
    }
{}

privilege_user::privilege_user(name::simple&& authorization_identifier, region_type region) :
    privilege_user {
            clone_unique(std::move(authorization_identifier)),
            region,
    }
{}


privilege_user::privilege_user(::takatori::util::clone_tag_t, privilege_user const& other) :
    privilege_user {
            clone_unique(*other.authorization_identifier_),
            other.region(),
    }
{}

privilege_user::privilege_user(::takatori::util::clone_tag_t, privilege_user&& other) :
    privilege_user {
            clone_unique(std::move(*other.authorization_identifier_)),
            other.region(),
    }
{}

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
    return eq(*a.authorization_identifier_, *b.authorization_identifier_);
}

bool operator!=(privilege_user const& a, privilege_user const& b) noexcept {
    return !(a == b);
}

::takatori::serializer::object_acceptor& operator<<(::takatori::serializer::object_acceptor& acceptor, privilege_user const& value) {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor);
    property(acceptor, "authorization_identifier"sv, *value.authorization_identifier_);
    region_property(acceptor, value);
    return acceptor;
}

std::ostream& operator<<(std::ostream& out, privilege_user const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::statement
