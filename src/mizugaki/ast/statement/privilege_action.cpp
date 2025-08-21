#include <mizugaki/ast/statement/privilege_action.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::statement {

privilege_action::privilege_action() noexcept :
        element {},
        action_kind_ { privilege_action_kind::all_privileges }
{}

privilege_action::privilege_action(action_kind_type action_kind, region_type region) noexcept :
        element { region },
        action_kind_ { action_kind }
{}

privilege_action::privilege_action(::takatori::util::clone_tag_t, privilege_action const& other) :
    privilege_action {
            other.action_kind_,
            other.region(),
    }
{}

privilege_action::privilege_action(::takatori::util::clone_tag_t, privilege_action&& other) :
    privilege_action {
            other.action_kind_,
            other.region(),
    }
{}

privilege_action::action_kind_type& privilege_action::action_kind() noexcept {
    return action_kind_;
}

privilege_action::action_kind_type const& privilege_action::action_kind() const noexcept {
    return action_kind_;
}

bool operator==(privilege_action const& a, privilege_action const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return false;
    }
    return eq(*a.action_kind_, *b.action_kind_);
}

bool operator!=(privilege_action const& a, privilege_action const& b) noexcept {
    return !(a == b);
}

::takatori::serializer::object_acceptor& operator<<(::takatori::serializer::object_acceptor& acceptor, privilege_action const& value) {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor);
    property(acceptor, "action_kind"sv, *value.action_kind_);
    region_property(acceptor, value);
    return acceptor;
}

std::ostream& operator<<(std::ostream& out, privilege_action const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::statement
