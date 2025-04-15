#include <mizugaki/ast/statement/column_definition.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>
#include <mizugaki/ast/common/vector.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::statement {

using ::takatori::util::clone_unique;

using common::clone_vector;

column_definition::column_definition(
        std::unique_ptr<name::simple> name,
        std::unique_ptr<type::type> type,
        std::vector<column_constraint_definition> constraints,
        region_type description,
        region_type region) noexcept :
    super { region },
    name_ { std::move(name) },
    type_ { std::move(type) },
    constraints_ { std::move(constraints) },
    description_ { description }
{}

column_definition::column_definition(
        name::simple&& name,
        type::type&& type,
        std::initializer_list<column_constraint_definition> constraints,
        region_type description,
        element::region_type region) :
    column_definition {
            clone_unique(std::move(name)),
            clone_unique(std::move(type)),
            constraints,
            description,
            region,
    }
{}

column_definition::column_definition(::takatori::util::clone_tag_t, column_definition const& other) :
    column_definition {
            clone_unique(other.name_),
            clone_unique(other.type_),
            clone_vector(other.constraints_),
            other.description_,
            other.region(),
    }
{}

column_definition::column_definition(::takatori::util::clone_tag_t, column_definition&& other) :
    column_definition {
            std::move(other.name_),
            std::move(other.type_),
            std::move(other.constraints_),
            other.description_,
            other.region(),
    }
{}

column_definition* column_definition::clone() const& {
    return new column_definition(::takatori::util::clone_tag, *this); // NOLINT
}

column_definition* column_definition::clone() && {
    return new column_definition(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

table_element::node_kind_type column_definition::node_kind() const noexcept {
    return tag;
}

std::unique_ptr<name::simple>& column_definition::name() noexcept {
    return name_;
}

std::unique_ptr<name::simple> const& column_definition::name() const noexcept {
    return name_;
}

std::unique_ptr<type::type>& column_definition::type() noexcept {
    return type_;
}

std::unique_ptr<type::type> const& column_definition::type() const noexcept {
    return type_;
}

std::vector<column_constraint_definition>& column_definition::constraints() noexcept {
    return constraints_;
}

std::vector<column_constraint_definition> const& column_definition::constraints() const noexcept {
    return constraints_;
}

column_definition::region_type& column_definition::description() noexcept {
    return description_;
}

column_definition::region_type const& column_definition::description() const noexcept {
    return description_;
}

bool operator==(column_definition const& a, column_definition const& b) noexcept {
    return eq(a.name_, b.name_)
        && eq(a.type_, b.type_)
        && eq(a.constraints_, b.constraints_);
}

bool operator!=(column_definition const& a, column_definition const& b) noexcept {
    return !(a == b);
}

bool column_definition::equals(table_element const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void column_definition::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "name"sv, name_);
    property(acceptor, "type"sv, type_);
    property(acceptor, "constraints"sv, constraints_);
    property(acceptor, "description"sv, description_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, column_definition const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::statement
