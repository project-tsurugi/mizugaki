#include <mizugaki/ast/statement/table_definition.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>
#include <mizugaki/ast/common/vector.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::statement {

using ::takatori::util::clone_unique;

using common::clone_vector;

table_definition::table_definition(
        std::unique_ptr<name::name> name,
        std::vector<std::unique_ptr<table_element>> elements,
        std::vector<option_type> options,
        std::vector<storage_parameter> parameters,
        region_type description,
        region_type region) noexcept :
    super { region },
    name_ { std::move(name) },
    elements_ { std::move(elements) },
    options_ { std::move(options) },
    parameters_ { std::move(parameters) },
    description_ { description }
{}

table_definition::table_definition(
        name::name&& name,
        common::rvalue_list<table_element> elements,
        std::initializer_list<option_type> options,
        std::initializer_list<storage_parameter> parameters,
        region_type description,
        region_type region) :
    table_definition {
            clone_unique(std::move(name)),
            common::to_vector(elements),
            options,
            parameters,
            description,
            region,
    }
{}

table_definition::table_definition(::takatori::util::clone_tag_t, table_definition const& other) :
    table_definition {
            clone_unique(other.name_),
            clone_vector(other.elements_),
            clone_vector(other.options_),
            clone_vector(other.parameters_),
            other.description_,
            other.region(),
    }
{}

table_definition::table_definition(::takatori::util::clone_tag_t, table_definition&& other) :
    table_definition {
            std::move(other.name_),
            std::move(other.elements_),
            std::move(other.options_),
            std::move(other.parameters_),
            other.description_,
            other.region(),
    }
{}

table_definition* table_definition::clone() const& {
    return new table_definition(::takatori::util::clone_tag, *this); // NOLINT
}

table_definition* table_definition::clone() && {
    return new table_definition(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

statement::node_kind_type table_definition::node_kind() const noexcept {
    return tag;
}

std::unique_ptr<name::name>& table_definition::name() noexcept {
    return name_;
}

std::unique_ptr<name::name> const& table_definition::name() const noexcept {
    return name_;
}

std::vector<std::unique_ptr<table_element>>& table_definition::elements() noexcept {
    return elements_;
}

std::vector<std::unique_ptr<table_element>> const& table_definition::elements() const noexcept {
    return elements_;
}

std::vector<table_definition::option_type>& table_definition::options() noexcept {
    return options_;
}

std::vector<table_definition::option_type> const& table_definition::options() const noexcept {
    return options_;
}

std::vector<storage_parameter>& table_definition::parameters() noexcept {
    return parameters_;
}

std::vector<storage_parameter> const& table_definition::parameters() const noexcept {
    return parameters_;
}

table_definition::region_type& table_definition::description() noexcept {
    return description_;
}

table_definition::region_type const& table_definition::description() const noexcept {
    return description_;
}

bool operator==(table_definition const& a, table_definition const& b) noexcept {
    // NOTE: never compare description_ here, because it is supplemental information for the definition.
    return eq(a.name_, b.name_)
        && eq(a.elements_, b.elements_)
        && eq(a.options_, b.options_)
        && eq(a.parameters_, b.parameters_);
}

bool operator!=(table_definition const& a, table_definition const& b) noexcept {
    return !(a == b);
}

bool table_definition::equals(statement const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void table_definition::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "name"sv, name_);
    property(acceptor, "elements"sv, elements_);
    property(acceptor, "options"sv, options_);
    property(acceptor, "parameters"sv, parameters_);
    property(acceptor, "description"sv, description_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, table_definition const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::statement
