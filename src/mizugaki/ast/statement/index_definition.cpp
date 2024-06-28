#include <mizugaki/ast/statement/index_definition.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>
#include <mizugaki/ast/common/vector.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::statement {

using ::takatori::util::clone_unique;

using common::clone_vector;

index_definition::index_definition(
        std::unique_ptr<name::name> name,
        std::unique_ptr<name::name> table_name,
        std::vector<common::sort_element> keys,
        std::vector<std::unique_ptr<scalar::expression>> values,
        std::unique_ptr<scalar::expression> predicate,
        std::vector<option_type> options,
        std::vector<storage_parameter> parameters,
        region_type region) noexcept :
    super { region },
    name_ { std::move(name) },
    table_name_ { std::move(table_name) },
    keys_ { std::move(keys) },
    values_ { std::move(values) },
    predicate_ { std::move(predicate) },
    options_ { std::move(options) },
    parameters_ { std::move(parameters) }
{}

index_definition::index_definition(
        ::takatori::util::rvalue_ptr<name::name> name,
        name::name&& table_name,
        std::initializer_list<common::sort_element> keys,
        common::rvalue_list<scalar::expression> values,
        ::takatori::util::rvalue_ptr<scalar::expression> predicate,
        std::initializer_list<option_type> options,
        std::initializer_list<storage_parameter> parameters,
        region_type region) :
    index_definition {
            clone_unique(name),
            clone_unique(std::move(table_name)),
            keys,
            common::to_vector(values),
            clone_unique(predicate),
            options,
            parameters,
            region,
    }
{}

index_definition::index_definition(::takatori::util::clone_tag_t, index_definition const& other) :
    index_definition {
            clone_unique(other.name_),
            clone_unique(other.table_name_),
            clone_vector(other.keys_),
            clone_vector(other.values_),
            clone_unique(other.predicate_),
            clone_vector(other.options_),
            clone_vector(other.parameters_),
            other.region(),
    }
{}

index_definition::index_definition(::takatori::util::clone_tag_t, index_definition&& other) :
    index_definition {
            std::move(other.name_),
            std::move(other.table_name_),
            std::move(other.keys_),
            std::move(other.values_),
            std::move(other.predicate_),
            std::move(other.options_),
            std::move(other.parameters_),
            other.region(),
    }
{}

index_definition* index_definition::clone() const& {
    return new index_definition(::takatori::util::clone_tag, *this); // NOLINT
}

index_definition* index_definition::clone() && {
    return new index_definition(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

statement::node_kind_type index_definition::node_kind() const noexcept {
    return tag;
}

std::unique_ptr<name::name>& index_definition::name() noexcept {
    return name_;
}

std::unique_ptr<name::name> const& index_definition::name() const noexcept {
    return name_;
}

std::unique_ptr<name::name>& index_definition::table_name() noexcept {
    return table_name_;
}

std::unique_ptr<name::name> const& index_definition::table_name() const noexcept {
    return table_name_;
}

std::vector<common::sort_element>& index_definition::keys() noexcept {
    return keys_;
}

std::vector<common::sort_element> const& index_definition::keys() const noexcept {
    return keys_;
}

std::vector<std::unique_ptr<scalar::expression>>& index_definition::values() noexcept {
    return values_;
}

std::vector<std::unique_ptr<scalar::expression>> const& index_definition::values() const noexcept {
    return values_;
}

std::unique_ptr<scalar::expression>& index_definition::predicate() noexcept {
    return predicate_;
}

std::unique_ptr<scalar::expression> const& index_definition::predicate() const noexcept {
    return predicate_;
}

std::vector<index_definition::option_type>& index_definition::options() noexcept {
    return options_;
}

std::vector<index_definition::option_type> const& index_definition::options() const noexcept {
    return options_;
}

std::vector<storage_parameter>& index_definition::parameters() noexcept {
    return parameters_;
}

std::vector<storage_parameter> const& index_definition::parameters() const noexcept {
    return parameters_;
}


bool operator==(index_definition const& a, index_definition const& b) noexcept {
    return eq(a.name_, b.name_)
        && eq(a.table_name_, b.table_name_)
        && eq(a.keys_, b.keys_)
        && eq(a.values_, b.values_)
        && eq(a.predicate_, b.predicate_)
        && eq(a.options_, b.options_)
        && eq(a.parameters_, b.parameters_);
}

bool operator!=(index_definition const& a, index_definition const& b) noexcept {
    return !(a == b);
}

bool index_definition::equals(statement const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void index_definition::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "name"sv, name_);
    property(acceptor, "table_name"sv, table_name_);
    property(acceptor, "keys"sv, keys_);
    property(acceptor, "values"sv, values_);
    property(acceptor, "predicate"sv, predicate_);
    property(acceptor, "options"sv, options_);
    property(acceptor, "parameters"sv, parameters_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, index_definition const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::statement
