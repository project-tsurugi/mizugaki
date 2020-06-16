#include <mizugaki/ast/common/sort_element.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::common {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

sort_element::sort_element(
        unique_object_ptr<scalar::expression> key,
        unique_object_ptr<name::name> collation,
        std::optional<direction_type> direction,
        element::region_type region) noexcept :
    element { region },
    key_ { std::move(key) },
    collation_ { std::move(collation) },
    direction_ { std::move(direction) }
{}

sort_element::sort_element(sort_element const& other, object_creator creator) :
    sort_element {
            clone_unique(*other.key_, creator),
            clone_unique(*other.collation_, creator),
            other.direction_,
            other.region(),
    }
{}

sort_element::sort_element(sort_element&& other, object_creator creator) :
    sort_element {
            clone_unique(std::move(*other.key_), creator),
            clone_unique(std::move(*other.collation_), creator),
            other.direction_,
            other.region(),
    }
{}

unique_object_ptr<scalar::expression>& sort_element::key() noexcept {
    return *key_;
}

unique_object_ptr<scalar::expression> const& sort_element::key() const noexcept {
    return *key_;
}

unique_object_ptr<name::name>& sort_element::collation() noexcept {
    return *collation_;
}

unique_object_ptr<name::name> const& sort_element::collation() const noexcept {
    return *collation_;
}

std::optional<sort_element::direction_type>& sort_element::direction() noexcept {
    return direction_;
}

std::optional<sort_element::direction_type> const& sort_element::direction() const noexcept {
    return direction_;
}

bool operator==(sort_element const& a, sort_element const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return false;
    }
    return eq(*a.key_, *b.key_)
        && eq(*a.collation_, *b.collation_)
        && eq(a.direction_, b.direction_);
}

bool operator!=(sort_element const& a, sort_element const& b) noexcept {
    return !(a == b);
}

} // namespace mizugaki::ast::common
