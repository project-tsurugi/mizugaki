#include <mizugaki/ast/statement/schema_definition.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>
#include <mizugaki/ast/common/vector.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::statement {

using ::takatori::util::clone_unique;

using common::clone_vector;

schema_definition::schema_definition(
        std::unique_ptr<name::name> name,
        std::unique_ptr<name::simple> user_name,
        std::vector<std::unique_ptr<statement>> elements,
        std::vector<option_type> options,
        region_type description,
        region_type region) noexcept :
    super { region },
    name_ { std::move(name) },
    user_name_ { std::move(user_name) },
    elements_ { std::move(elements) },
    options_ { std::move(options) },
    description_ { description }
{}

schema_definition::schema_definition(
        ::takatori::util::rvalue_ptr<name::name> name,
        ::takatori::util::rvalue_ptr<name::simple> user_name,
        common::rvalue_list<statement> elements,
        std::initializer_list<option_type> options,
        region_type description,
        region_type region) :
    schema_definition {
            clone_unique(name),
            clone_unique(user_name),
            common::to_vector(elements),
            options,
            description,
            region,
    }
{}

schema_definition::schema_definition(::takatori::util::clone_tag_t, schema_definition const& other) :
    schema_definition {
            clone_unique(other.name_),
            clone_unique(other.user_name_),
            clone_vector(other.elements_),
            clone_vector(other.options_),
            other.description_,
            other.region(),
    }
{}

schema_definition::schema_definition(::takatori::util::clone_tag_t, schema_definition&& other) :
    schema_definition {
            std::move(other.name_),
            std::move(other.user_name_),
            std::move(other.elements_),
            std::move(other.options_),
            other.description_,
            other.region(),
    }
{}

schema_definition* schema_definition::clone() const& {
    return new schema_definition(::takatori::util::clone_tag, *this); // NOLINT
}

schema_definition* schema_definition::clone() && {
    return new schema_definition(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

statement::node_kind_type schema_definition::node_kind() const noexcept {
    return tag;
}

std::unique_ptr<name::name>& schema_definition::name() noexcept {
    return name_;
}

std::unique_ptr<name::name> const& schema_definition::name() const noexcept {
    return name_;
}

std::unique_ptr<name::simple>& schema_definition::user_name() noexcept {
    return user_name_;
}

std::unique_ptr<name::simple> const& schema_definition::user_name() const noexcept {
    return user_name_;
}

std::vector<std::unique_ptr<statement>>& schema_definition::elements() noexcept {
    return elements_;
}

std::vector<std::unique_ptr<statement>> const& schema_definition::elements() const noexcept {
    return elements_;
}

std::vector<schema_definition::option_type>& schema_definition::options() noexcept {
    return options_;
}

std::vector<schema_definition::option_type> const& schema_definition::options() const noexcept {
    return options_;
}

schema_definition::region_type& schema_definition::description() noexcept {
    return description_;
}

schema_definition::region_type const& schema_definition::description() const noexcept {
    return description_;
}

bool operator==(schema_definition const& a, schema_definition const& b) noexcept {
    return eq(a.name_, b.name_)
        && eq(a.user_name_, b.user_name_)
        && eq(a.elements_, b.elements_)
        && eq(a.options_, b.options_);
}

bool operator!=(schema_definition const& a, schema_definition const& b) noexcept {
    return !(a == b);
}

bool schema_definition::equals(statement const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void schema_definition::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "name"sv, name_);
    property(acceptor, "user_name"sv, user_name_);
    property(acceptor, "elements"sv, elements_);
    property(acceptor, "options"sv, options_);
    property(acceptor, "description"sv, description_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, schema_definition const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::statement
