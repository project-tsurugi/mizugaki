#include <mizugaki/ast/common/target_element.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::common {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

target_element::target_element(
        unique_object_ptr<name::name> target,
        unique_object_ptr<name::simple> indicator,
        region_type region) noexcept :
    element { region },
    target_ { std::move(target) },
    indicator_ { std::move(indicator) }
{}

target_element::target_element(target_element const& other, object_creator creator) :
    target_element {
            clone_unique(*other.target_, creator),
            clone_unique(*other.indicator_, creator),
            other.region(),
    }
{}

target_element::target_element(target_element&& other, object_creator creator) :
    target_element {
            clone_unique(std::move(*other.target_), creator),
            clone_unique(std::move(*other.indicator_), creator),
            other.region(),
    }
{}

unique_object_ptr<name::name>& target_element::target() noexcept {
    return *target_;
}

unique_object_ptr<name::name> const& target_element::target() const noexcept {
    return *target_;
}

unique_object_ptr<name::simple>& target_element::indicator() noexcept {
    return *indicator_;
}

unique_object_ptr<name::simple> const& target_element::indicator() const noexcept {
    return *indicator_;
}

bool operator==(target_element const& a, target_element const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return false;
    }
    return eq(*a.target_, *b.target_)
            && eq(*a.indicator_, *b.indicator_);
}

bool operator!=(target_element const& a, target_element const& b) noexcept {
    return !(a == b);
}

::takatori::serializer::object_acceptor& operator<<(::takatori::serializer::object_acceptor& acceptor, target_element const& value) {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor);
    property(acceptor, "target"sv, *value.target_);
    property(acceptor, "indicator"sv, *value.indicator_);
    region_property(acceptor, value);
    return acceptor;
}

std::ostream& operator<<(std::ostream& out, target_element const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::common
