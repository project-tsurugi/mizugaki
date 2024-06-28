#include <mizugaki/ast/statement/storage_parameter.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::statement {

using ::takatori::util::clone_unique;
using ::takatori::util::rvalue_ptr;

storage_parameter::storage_parameter(
        std::unique_ptr<name::name> name,
        std::unique_ptr<scalar::expression> value,
        region_type region) noexcept :
    element { region },
    name_ { std::move(name) },
    value_ { std::move(value) }
{}

storage_parameter::storage_parameter(
        name::name&& name,
        rvalue_ptr<scalar::expression> value,
        region_type region) :
    storage_parameter {
            clone_unique(std::move(name)),
            clone_unique(value),
            region,
    }
{}

storage_parameter::storage_parameter(::takatori::util::clone_tag_t, storage_parameter const& other) :
    storage_parameter {
            clone_unique(*other.name_),
            clone_unique(*other.value_),
            other.region(),
    }
{}

storage_parameter::storage_parameter(::takatori::util::clone_tag_t, storage_parameter&& other) :
    storage_parameter {
            clone_unique(std::move(*other.name_)),
            clone_unique(std::move(*other.value_)),
            other.region(),
    }
{}

std::unique_ptr<name::name>& storage_parameter::name() noexcept {
    return *name_;
}

std::unique_ptr<name::name> const& storage_parameter::name() const noexcept {
    return *name_;
}

std::unique_ptr<scalar::expression>& storage_parameter::value() noexcept {
    return *value_;
}

std::unique_ptr<scalar::expression> const& storage_parameter::value() const noexcept {
    return *value_;
}

bool operator==(storage_parameter const& a, storage_parameter const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return false;
    }
    return eq(*a.name_, *b.name_)
            && eq(*a.value_, *b.value_);
}

bool operator!=(storage_parameter const& a, storage_parameter const& b) noexcept {
    return !(a == b);
}

::takatori::serializer::object_acceptor& operator<<(::takatori::serializer::object_acceptor& acceptor, storage_parameter const& value) {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor);
    property(acceptor, "name"sv, *value.name_);
    property(acceptor, "value"sv, *value.value_);
    region_property(acceptor, value);
    return acceptor;
}

std::ostream& operator<<(std::ostream& out, storage_parameter const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::statement
