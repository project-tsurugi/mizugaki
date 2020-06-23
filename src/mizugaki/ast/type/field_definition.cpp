#include <mizugaki/ast/type/field_definition.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::type {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::rvalue_ptr;
using ::takatori::util::unique_object_ptr;

field_definition::field_definition(
        unique_object_ptr<name::simple> name,
        unique_object_ptr<ast::type::type> type,
        unique_object_ptr<name::name> collation,
        region_type region) noexcept :
    element { region },
    name_ { std::move(name) },
    type_ { std::move(type) },
    collation_ { std::move(collation) }
{}

field_definition::field_definition(
        name::simple&& name,
        ast::type::type&& type,
        rvalue_ptr<name::name> collation,
        element::region_type region) :
    field_definition {
            clone_unique(std::move(name)),
            clone_unique(std::move(type)),
            clone_unique(collation),
            region,
    }
{}

field_definition::field_definition(field_definition const& other, object_creator creator) :
    field_definition {
            clone_unique(*other.name_, creator),
            clone_unique(*other.type_, creator),
            clone_unique(*other.collation_, creator),
            other.region(),
    }
{}

field_definition::field_definition(field_definition&& other, object_creator creator) :
    field_definition {
            clone_unique(std::move(*other.name_), creator),
            clone_unique(std::move(*other.type_), creator),
            clone_unique(std::move(*other.collation_), creator),
            other.region(),
    }
{}

unique_object_ptr<name::simple>& field_definition::name() noexcept {
    return *name_;
}

unique_object_ptr<name::simple> const& field_definition::name() const noexcept {
    return *name_;
}

unique_object_ptr<ast::type::type>& field_definition::type() noexcept {
    return *type_;
}

unique_object_ptr<ast::type::type> const& field_definition::type() const noexcept {
    return *type_;
}

unique_object_ptr<name::name>& field_definition::collation() noexcept {
    return *collation_;
}

unique_object_ptr<name::name> const& field_definition::collation() const noexcept {
    return *collation_;
}

bool operator==(field_definition const& a, field_definition const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return false;
    }
    return eq(*a.name_, *b.name_)
        && eq(*a.type_, *b.type_)
        && eq(*a.collation_, *b.collation_);
}

bool operator!=(field_definition const& a, field_definition const& b) noexcept {
    return !(a == b);
}

::takatori::serializer::object_acceptor& operator<<(takatori::serializer::object_acceptor& acceptor, field_definition const& value) {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor);
    property(acceptor, "name"sv, *value.name_);
    property(acceptor, "type"sv, *value.type_);
    property(acceptor, "collation"sv, *value.collation_);
    region_property(acceptor, value);
    return acceptor;
}

std::ostream& operator<<(std::ostream& out, field_definition const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::type
