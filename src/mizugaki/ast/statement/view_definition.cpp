#include <mizugaki/ast/statement/view_definition.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>
#include <mizugaki/ast/common/vector.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::statement {

using ::takatori::util::clone_unique;

using common::clone_vector;

view_definition::view_definition(
        std::unique_ptr<name::name> name,
        std::vector<std::unique_ptr<name::simple>> columns,
        std::unique_ptr<query::expression> query,
        std::vector<option_type> options,
        std::vector<storage_parameter> parameters,
        region_type description,
        region_type region) noexcept :
    super { region },
    name_ { std::move(name) },
    columns_ { std::move(columns) },
    query_ { std::move(query) },
    options_ { std::move(options) },
    parameters_ { std::move(parameters) },
    description_ { description }
{}

view_definition::view_definition(
        name::name&& name,
        common::rvalue_list<name::simple> columns,
        query::expression&& query,
        std::initializer_list<option_type> options,
        std::initializer_list<storage_parameter> parameters,
        region_type description,
        region_type region) :
    view_definition {
            clone_unique(std::move(name)),
            common::to_vector(columns),
            clone_unique(std::move(query)),
            options,
            parameters,
            description,
            region,
    }
{}

view_definition::view_definition(::takatori::util::clone_tag_t, view_definition const& other) :
    view_definition {
            clone_unique(other.name_),
            clone_vector(other.columns_),
            clone_unique(other.query_),
            clone_vector(other.options_),
            clone_vector(other.parameters_),
            other.description_,
            other.region(),
    }
{}

view_definition::view_definition(::takatori::util::clone_tag_t, view_definition&& other) :
    view_definition {
            std::move(other.name_),
            std::move(other.columns_),
            std::move(other.query_),
            std::move(other.options_),
            std::move(other.parameters_),
            other.description_,
            other.region(),
    }
{}

view_definition* view_definition::clone() const& {
    return new view_definition(::takatori::util::clone_tag, *this); // NOLINT
}

view_definition* view_definition::clone() && {
    return new view_definition(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

statement::node_kind_type view_definition::node_kind() const noexcept {
    return tag;
}

std::unique_ptr<name::name>& view_definition::name() noexcept {
    return name_;
}

std::unique_ptr<name::name> const& view_definition::name() const noexcept {
    return name_;
}

std::vector<std::unique_ptr<name::simple>>& view_definition::columns() noexcept {
    return columns_;
}

std::vector<std::unique_ptr<name::simple>> const& view_definition::columns() const noexcept {
    return columns_;
}

std::unique_ptr<query::expression>& view_definition::query() noexcept {
    return query_;
}

std::unique_ptr<query::expression> const& view_definition::query() const noexcept {
    return query_;
}

std::vector<view_definition::option_type>& view_definition::options() noexcept {
    return options_;
}

std::vector<view_definition::option_type> const& view_definition::options() const noexcept {
    return options_;
}

std::vector<storage_parameter>& view_definition::parameters() noexcept {
    return parameters_;
}

std::vector<storage_parameter> const& view_definition::parameters() const noexcept {
    return parameters_;
}

view_definition::region_type& view_definition::description() noexcept {
    return description_;
}

view_definition::region_type const& view_definition::description() const noexcept {
    return description_;
}

bool operator==(view_definition const& a, view_definition const& b) noexcept {
    return eq(a.name_, b.name_)
        && eq(a.columns_, b.columns_)
        && eq(a.query_, b.query_)
        && eq(a.options_, b.options_)
        && eq(a.parameters_, b.parameters_);
}

bool operator!=(view_definition const& a, view_definition const& b) noexcept {
    return !(a == b);
}

bool view_definition::equals(statement const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void view_definition::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "name"sv, name_);
    property(acceptor, "columns"sv, columns_);
    property(acceptor, "query"sv, query_);
    property(acceptor, "options"sv, options_);
    property(acceptor, "parameters"sv, parameters_);
    property(acceptor, "description"sv, description_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, view_definition const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::statement
