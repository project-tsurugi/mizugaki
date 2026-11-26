#include <mizugaki/ast/statement/partition_key_element.h>

#include <mizugaki/ast/common/serializers.h>
#include <mizugaki/ast/scalar/variable_reference.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::statement {

using ::takatori::util::clone_unique;
using ::takatori::util::rvalue_ptr;

partition_key_element::partition_key_element(
        std::unique_ptr<scalar::expression> value,
        std::unique_ptr<name::simple> name,
        region_type region) noexcept :
    element { region },
    value_ { std::move(value) },
    name_ { std::move(name) }
{}

partition_key_element::partition_key_element(
        scalar::expression&& value,
        rvalue_ptr<name::simple> name,
        region_type region) :
    partition_key_element {
            clone_unique(std::move(value)),
            clone_unique(name),
            region,
    }
{}

static inline std::unique_ptr<scalar::variable_reference> to_expr(name::name&& column) {
    auto r = column.region();
    return std::make_unique<scalar::variable_reference>(std::move(column), r);
}

partition_key_element::partition_key_element(
        name::name&& column,
        region_type region) :
    partition_key_element {
            to_expr(std::move(column)),
            nullptr,
            region,
    }
{}

partition_key_element::partition_key_element(::takatori::util::clone_tag_t, partition_key_element const& other) :
    partition_key_element {
            clone_unique(*other.value_),
            clone_unique(*other.name_),
            other.region(),
    }
{}

partition_key_element::partition_key_element(::takatori::util::clone_tag_t, partition_key_element&& other) :
    partition_key_element {
            clone_unique(std::move(*other.value_)),
            clone_unique(std::move(*other.name_)),
            other.region(),
    }
{}

std::unique_ptr<scalar::expression>& partition_key_element::value() noexcept {
    return *value_;
}

std::unique_ptr<scalar::expression> const& partition_key_element::value() const noexcept {
    return *value_;
}

std::unique_ptr<name::simple>& partition_key_element::name() noexcept {
    return *name_;
}

std::unique_ptr<name::simple> const& partition_key_element::name() const noexcept {
    return *name_;
}

bool operator==(partition_key_element const& a, partition_key_element const& b) noexcept {
    return eq(*a.value_, *b.value_)
            && eq(*a.name_, *b.name_);
}

bool operator!=(partition_key_element const& a, partition_key_element const& b) noexcept {
    return !(a == b);
}

::takatori::serializer::object_acceptor& operator<<(
        ::takatori::serializer::object_acceptor& acceptor,
        partition_key_element const& value) {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor);
    property(acceptor, "value"sv, *value.value_);
    property(acceptor, "name"sv, *value.name_);
    region_property(acceptor, value);
    return acceptor;
}

std::ostream& operator<<(std::ostream& out, partition_key_element const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::statement
