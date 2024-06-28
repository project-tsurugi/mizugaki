#include <mizugaki/ast/statement/key_constraint.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>
#include <mizugaki/ast/common/vector.h>

#include "utils.h"

namespace mizugaki::ast::statement {

using common::clone_vector;

key_constraint::key_constraint(
        constraint_kind_type constraint_kind,
        std::vector<common::sort_element> key,
        std::vector<std::unique_ptr<scalar::expression>> values,
        std::vector<storage_parameter> parameters,
        region_type region) :
    super { region },
    constraint_kind_ { constraint_kind },
    key_ { std::move(key) },
    values_ { std::move(values) },
    parameters_ { std::move(parameters) }
{
    utils::validate_kind(tags, *constraint_kind);
}

key_constraint::key_constraint(
        constraint_kind_type constraint_kind,
        std::initializer_list<common::sort_element> key,
        common::rvalue_list<scalar::expression> values,
        std::initializer_list<storage_parameter> parameters,
        region_type region) :
    key_constraint {
        constraint_kind,
        key,
        common::to_vector(values),
        parameters,
        region,
    }
{}

key_constraint::key_constraint(::takatori::util::clone_tag_t, key_constraint const& other) :
    key_constraint {
            other.constraint_kind_,
            clone_vector(other.key_),
            clone_vector(other.values_),
            clone_vector(other.parameters_),
            other.region(),
    }
{}

key_constraint::key_constraint(::takatori::util::clone_tag_t, key_constraint&& other) :
        key_constraint {
                other.constraint_kind_,
                std::move(other.key_),
                std::move(other.values_),
                std::move(other.parameters_),
                other.region(),
        }
{}

key_constraint* key_constraint::clone() const& {
    return new key_constraint(::takatori::util::clone_tag, *this); // NOLINT
}

key_constraint* key_constraint::clone() && {
    return new key_constraint(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

constraint::node_kind_type key_constraint::node_kind() const noexcept {
    return *constraint_kind_;
}

key_constraint::constraint_kind_type& key_constraint::constraint_kind() noexcept {
    return constraint_kind_;
}

key_constraint::constraint_kind_type const& key_constraint::constraint_kind() const noexcept {
    return constraint_kind_;
}

std::vector<common::sort_element>& key_constraint::key() noexcept {
    return key_;
}
    
std::vector<common::sort_element> const& key_constraint::key() const noexcept {
    return key_;
}
    
std::vector<std::unique_ptr<scalar::expression>>& key_constraint::values() noexcept {
    return values_;
}
    
std::vector<std::unique_ptr<scalar::expression>> const& key_constraint::values() const noexcept {
    return values_;
}
    
std::vector<storage_parameter>& key_constraint::parameters() noexcept {
    return parameters_;
}
    
std::vector<storage_parameter> const& key_constraint::parameters() const noexcept {
    return parameters_;
}

bool operator==(key_constraint const& a, key_constraint const& b) noexcept {
    return eq(a.constraint_kind_, b.constraint_kind_)
        && eq(a.key_, b.key_)
        && eq(a.values_, b.values_)
        && eq(a.parameters_, b.parameters_);
}

bool operator!=(key_constraint const& a, key_constraint const& b) noexcept {
    return !(a == b);
}

bool key_constraint::equals(constraint const& other) const noexcept {
    return tags.contains(other.node_kind())
            && *this == unsafe_downcast<key_constraint>(other);
}

void key_constraint::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "constraint_kind"sv, constraint_kind_);
    property(acceptor, "key"sv, key_);
    property(acceptor, "values"sv, values_);
    property(acceptor, "parameters"sv, parameters_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, key_constraint const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::statement
