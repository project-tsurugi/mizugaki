#include <mizugaki/ast/statement/set_element.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::statement {

using ::takatori::util::clone_unique;

set_element::set_element(
        std::unique_ptr<name::name> target,
        std::unique_ptr<scalar::expression> value,
        region_type region) noexcept :
    element { region },
    target_ { std::move(target) },
    value_ { std::move(value) }
{}

set_element::set_element(
        name::name&& target,
        scalar::expression&& value,
        region_type region) :
    set_element {
            clone_unique(std::move(target)),
            clone_unique(std::move(value)),
            region,
    }
{}

set_element::set_element(::takatori::util::clone_tag_t, set_element const& other) :
    set_element {
            clone_unique(*other.target_),
            clone_unique(*other.value_),
            other.region(),
    }
{}

set_element::set_element(::takatori::util::clone_tag_t, set_element&& other) :
    set_element {
            clone_unique(std::move(*other.target_)),
            clone_unique(std::move(*other.value_)),
            other.region(),
    }
{}

std::unique_ptr<name::name>& set_element::target() noexcept {
    return *target_;
}

std::unique_ptr<name::name> const& set_element::target() const noexcept {
    return *target_;
}

std::unique_ptr<scalar::expression>& set_element::value() noexcept {
    return *value_;
}

std::unique_ptr<scalar::expression> const& set_element::value() const noexcept {
    return *value_;
}

bool operator==(set_element const& a, set_element const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return false;
    }
    return eq(*a.target_, *b.target_)
            && eq(*a.value_, *b.value_);
}

bool operator!=(set_element const& a, set_element const& b) noexcept {
    return !(a == b);
}

::takatori::serializer::object_acceptor& operator<<(::takatori::serializer::object_acceptor& acceptor, set_element const& value) {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor);
    property(acceptor, "target"sv, *value.target_);
    property(acceptor, "value"sv, *value.value_);
    region_property(acceptor, value);
    return acceptor;
}

std::ostream& operator<<(std::ostream& out, set_element const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::statement
