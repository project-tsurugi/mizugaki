#include <mizugaki/ast/statement/set_element.h>

namespace mizugaki::ast::statement {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

set_element::set_element(
        unique_object_ptr<name::name> target,
        unique_object_ptr<scalar::expression> value,
        region_type region) noexcept :
    element { region },
    target_ { std::move(target) },
    value_ { std::move(value) }
{}

set_element::set_element(set_element const& other, object_creator creator) :
    set_element {
            clone_unique(*other.target_, creator),
            clone_unique(*other.value_, creator),
            other.region(),
    }
{}

set_element::set_element(set_element&& other, object_creator creator) :
    set_element {
            clone_unique(std::move(*other.target_), creator),
            clone_unique(std::move(*other.value_), creator),
            other.region(),
    }
{}

unique_object_ptr<name::name>& set_element::target() noexcept {
    return *target_;
}

unique_object_ptr<name::name> const& set_element::target() const noexcept {
    return *target_;
}

unique_object_ptr<scalar::expression>& set_element::value() noexcept {
    return *value_;
}

unique_object_ptr<scalar::expression> const& set_element::value() const noexcept {
    return *value_;
}

} // namespace mizugaki::ast::statement
