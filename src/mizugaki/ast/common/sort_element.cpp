#include <mizugaki/ast/common/sort_element.h>

#include <mizugaki/ast/common/serializers.h>
#include <mizugaki/ast/scalar/variable_reference.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::common {

using ::takatori::util::clone_unique;
using ::takatori::util::rvalue_ptr;

sort_element::sort_element(
        std::unique_ptr<scalar::expression> key,
        std::unique_ptr<name::name> collation,
        std::optional<direction_type> direction,
        element::region_type region) noexcept :
    element { region },
    key_ { std::move(key) },
    collation_ { std::move(collation) },
    direction_ { direction }
{}

sort_element::sort_element(
        scalar::expression&& key,
        rvalue_ptr<name::name> collation,
        std::optional<direction_type> direction,
        region_type region) :
    sort_element {
            clone_unique(std::move(key)),
            clone_unique(collation),
            direction,
            region,
    }
{}

sort_element::sort_element(
        scalar::expression&& key,
        direction_type direction,
        region_type region) noexcept :
    sort_element {
            clone_unique(std::move(key)),
            nullptr,
            direction,
            region,
    }
{}

static inline std::unique_ptr<scalar::variable_reference> to_expr(name::name&& column) {
    auto r = column.region();
    return std::make_unique<scalar::variable_reference>(std::move(column), r);
}

sort_element::sort_element(
        name::name&& key,
        rvalue_ptr<name::name> collation,
        std::optional<direction_type> direction,
        region_type region) noexcept :
    sort_element {
            to_expr(std::move(key)),
            clone_unique(collation),
            direction,
            region,
    }
{}

sort_element::sort_element(
        name::name&& key,
        direction_type direction,
        region_type region) noexcept :
    sort_element {
            to_expr(std::move(key)),
            nullptr,
            direction,
            region,
    }
{}

sort_element::sort_element(::takatori::util::clone_tag_t, sort_element const& other) :
    sort_element {
            clone_unique(*other.key_),
            clone_unique(*other.collation_),
            other.direction_,
            other.region(),
    }
{}

sort_element::sort_element(::takatori::util::clone_tag_t, sort_element&& other) :
    sort_element {
            clone_unique(std::move(*other.key_)),
            clone_unique(std::move(*other.collation_)),
            other.direction_,
            other.region(),
    }
{}

std::unique_ptr<scalar::expression>& sort_element::key() noexcept {
    return *key_;
}

std::unique_ptr<scalar::expression> const& sort_element::key() const noexcept {
    return *key_;
}

std::unique_ptr<name::name>& sort_element::collation() noexcept {
    return *collation_;
}

std::unique_ptr<name::name> const& sort_element::collation() const noexcept {
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

::takatori::serializer::object_acceptor& operator<<(takatori::serializer::object_acceptor& acceptor, sort_element const& value) {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor);
    property(acceptor, "key"sv, *value.key_);
    property(acceptor, "collation"sv, *value.collation_);
    property(acceptor, "direction"sv, value.direction_);
    region_property(acceptor, value);
    return acceptor;
}

std::ostream& operator<<(std::ostream& out, sort_element const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::common
