#include <mizugaki/ast/statement/table_constraint_definition.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>
#include <mizugaki/ast/common/vector.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::statement {

using ::takatori::util::clone_unique;
using ::takatori::util::rvalue_ptr;

table_constraint_definition::table_constraint_definition(
        std::unique_ptr<name::name> name,
        std::unique_ptr<constraint> body,
        region_type region) noexcept :
    super { region },
    name_ { std::move(name) },
    body_ { std::move(body) }
{}

table_constraint_definition::table_constraint_definition(
        constraint&& body,
        ::takatori::util::rvalue_ptr<name::name> name,
        element::region_type region) :
    table_constraint_definition {
            clone_unique(name),
            clone_unique(std::move(body)),
            region,
    }
{}

table_constraint_definition::table_constraint_definition(::takatori::util::clone_tag_t, table_constraint_definition const& other) :
    table_constraint_definition {
            clone_unique(other.name_),
            clone_unique(other.body_),
            other.region(),
    }
{}

table_constraint_definition::table_constraint_definition(::takatori::util::clone_tag_t, table_constraint_definition&& other) :
    table_constraint_definition {
            std::move(other.name_),
            std::move(other.body_),
            other.region(),
    }
{}

table_constraint_definition* table_constraint_definition::clone() const& {
    return new table_constraint_definition(::takatori::util::clone_tag, *this); // NOLINT
}

table_constraint_definition* table_constraint_definition::clone() && {
    return new table_constraint_definition(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

table_element::node_kind_type table_constraint_definition::node_kind() const noexcept {
    return tag;
}

std::unique_ptr<name::name>& table_constraint_definition::name() noexcept {
    return name_;
}

std::unique_ptr<name::name> const& table_constraint_definition::name() const noexcept {
    return name_;
}

std::unique_ptr<constraint>& table_constraint_definition::body() noexcept {
    return body_;
}

std::unique_ptr<constraint> const& table_constraint_definition::body() const noexcept {
    return body_;
}

bool operator==(table_constraint_definition const& a, table_constraint_definition const& b) noexcept {
    return eq(a.name_, b.name_)
        && eq(a.body_, b.body_);
}

bool operator!=(table_constraint_definition const& a, table_constraint_definition const& b) noexcept {
    return !(a == b);
}

bool table_constraint_definition::equals(table_element const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void table_constraint_definition::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "name"sv, name_);
    property(acceptor, "body"sv, body_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, table_constraint_definition const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::statement
