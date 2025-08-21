#include <mizugaki/ast/statement/privilege_object.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

#include "mizugaki/ast/statement/privilege_action_kind.h"

namespace mizugaki::ast::statement {

using ::takatori::util::clone_unique;

privilege_object::privilege_object() noexcept :
    element {},
    object_kind_ {},
    object_name_ { nullptr }
{}

privilege_object::privilege_object(
        std::optional<object_kind_type> object_kind,
        std::unique_ptr<name::name> object_name,
        region_type region) noexcept :
    element { region },
    object_kind_ { object_kind },
    object_name_ { std::move(object_name) }
{}

privilege_object::privilege_object(name::name&& object_name, region_type region) :
    privilege_object {
            std::nullopt,
            clone_unique(std::move(object_name)),
            region,
    }
{}

privilege_object::privilege_object(object_kind_type object_kind, name::name&& object_name, region_type region) :
    privilege_object {
            object_kind,
            clone_unique(std::move(object_name)),
            region,
    }
{}

privilege_object::privilege_object(::takatori::util::clone_tag_t, privilege_object const& other) :
    privilege_object {
            other.object_kind_,
            clone_unique(*other.object_name_),
            other.region(),
    }
{}

privilege_object::privilege_object(::takatori::util::clone_tag_t, privilege_object&& other) :
    privilege_object {
            other.object_kind_,
            clone_unique(std::move(*other.object_name_)),
            other.region(),
    }
{}

std::optional<privilege_object::object_kind_type>& privilege_object::object_kind() noexcept {
    return object_kind_;
}

std::optional<privilege_object::object_kind_type> const& privilege_object::object_kind() const noexcept {
    return object_kind_;
}

std::unique_ptr<name::name>& privilege_object::object_name() noexcept {
    return *object_name_;
}

std::unique_ptr<name::name> const& privilege_object::object_name() const noexcept {
    return *object_name_;
}

bool operator==(privilege_object const& a, privilege_object const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return false;
    }
    return eq(a.object_kind_, b.object_kind_)
            && eq(*a.object_name_, *b.object_name_);
}

bool operator!=(privilege_object const& a, privilege_object const& b) noexcept {
    return !(a == b);
}

::takatori::serializer::object_acceptor& operator<<(::takatori::serializer::object_acceptor& acceptor, privilege_object const& value) {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor);
    property(acceptor, "object_kind"sv, value.object_kind_);
    property(acceptor, "object_name"sv, *value.object_name_);
    region_property(acceptor, value);
    return acceptor;
}

std::ostream& operator<<(std::ostream& out, privilege_object const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::statement
