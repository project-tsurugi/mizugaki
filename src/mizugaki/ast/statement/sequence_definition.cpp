#include <mizugaki/ast/statement/sequence_definition.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>
#include <mizugaki/ast/common/vector.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::statement {

using ::takatori::util::clone_unique;

using common::clone_vector;

sequence_definition::sequence_definition(
        std::unique_ptr<name::name> name,
        std::unique_ptr<type::type> type,
        std::unique_ptr<scalar::expression> initial_value,
        std::unique_ptr<scalar::expression> increment_value,
        std::unique_ptr<scalar::expression> min_value,
        std::unique_ptr<scalar::expression> max_value,
        std::unique_ptr<name::name> owner,
        std::vector<option_type> options,
        region_type region) noexcept :
    super { region },
    name_ { std::move(name) },
    type_ { std::move(type) },
    initial_value_ { std::move(initial_value) },
    increment_value_ { std::move(increment_value) },
    min_value_ { std::move(min_value) },
    max_value_ { std::move(max_value) },
    owner_ { std::move(owner) },
    options_ { std::move(options) }
{}

sequence_definition::sequence_definition(
        name::name&& name,
        ::takatori::util::rvalue_ptr<type::type> type,
        ::takatori::util::rvalue_ptr<scalar::expression> initial_value,
        ::takatori::util::rvalue_ptr<scalar::expression> increment_value,
        ::takatori::util::rvalue_ptr<scalar::expression> min_value,
        ::takatori::util::rvalue_ptr<scalar::expression> max_value,
        ::takatori::util::rvalue_ptr<name::name> owner,
        std::initializer_list<option_type> options,
        region_type region) :
    sequence_definition {
            clone_unique(std::move(name)),
            clone_unique(type),
            clone_unique(initial_value),
            clone_unique(increment_value),
            clone_unique(min_value),
            clone_unique(max_value),
            clone_unique(owner),
            options,
            region,
    }
{}

sequence_definition::sequence_definition(::takatori::util::clone_tag_t, sequence_definition const& other) :
    sequence_definition {
            clone_unique(other.name_),
            clone_unique(other.type_),
            clone_unique(other.initial_value_),
            clone_unique(other.increment_value_),
            clone_unique(other.min_value_),
            clone_unique(other.max_value_),
            clone_unique(other.owner_),
            clone_vector(other.options_),
            other.region(),
    }
{}

sequence_definition::sequence_definition(::takatori::util::clone_tag_t, sequence_definition&& other) :
    sequence_definition {
            std::move(other.name_),
            std::move(other.type_),
            std::move(other.initial_value_),
            std::move(other.increment_value_),
            std::move(other.min_value_),
            std::move(other.max_value_),
            std::move(other.owner_),
            std::move(other.options_),
            other.region(),
    }
{}

sequence_definition* sequence_definition::clone() const& {
    return new sequence_definition(::takatori::util::clone_tag, *this); // NOLINT
}

sequence_definition* sequence_definition::clone() && {
    return new sequence_definition(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

statement::node_kind_type sequence_definition::node_kind() const noexcept {
    return tag;
}

std::unique_ptr<name::name>& sequence_definition::name() noexcept {
    return name_;
}

std::unique_ptr<name::name> const& sequence_definition::name() const noexcept {
    return name_;
}

std::unique_ptr<type::type>& sequence_definition::type() noexcept {
    return type_;
}

std::unique_ptr<type::type> const& sequence_definition::type() const noexcept {
    return type_;
}

std::unique_ptr<scalar::expression>& sequence_definition::initial_value() noexcept {
    return initial_value_;
}

std::unique_ptr<scalar::expression> const& sequence_definition::initial_value() const noexcept {
    return initial_value_;
}

std::unique_ptr<scalar::expression>& sequence_definition::increment_value() noexcept {
    return increment_value_;
}

std::unique_ptr<scalar::expression> const& sequence_definition::increment_value() const noexcept {
    return increment_value_;
}

std::unique_ptr<scalar::expression>& sequence_definition::min_value() noexcept {
    return min_value_;
}

std::unique_ptr<scalar::expression> const& sequence_definition::min_value() const noexcept {
    return min_value_;
}

std::unique_ptr<scalar::expression>& sequence_definition::max_value() noexcept {
    return max_value_;
}

std::unique_ptr<scalar::expression> const& sequence_definition::max_value() const noexcept {
    return max_value_;
}

std::unique_ptr<name::name>& sequence_definition::owner() noexcept {
    return owner_;
}

std::unique_ptr<name::name> const& sequence_definition::owner() const noexcept {
    return owner_;
}

std::vector<sequence_definition::option_type>& sequence_definition::options() noexcept {
    return options_;
}

std::vector<sequence_definition::option_type> const& sequence_definition::options() const noexcept {
    return options_;
}

bool operator==(sequence_definition const& a, sequence_definition const& b) noexcept {
    return eq(a.name_, b.name_)
        && eq(a.type_, b.type_)
        && eq(a.initial_value_, b.initial_value_)
        && eq(a.increment_value_, b.increment_value_)
        && eq(a.min_value_, b.min_value_)
        && eq(a.max_value_, b.max_value_)
        && eq(a.owner_, b.owner_)
        && eq(a.options_, b.options_);
}

bool operator!=(sequence_definition const& a, sequence_definition const& b) noexcept {
    return !(a == b);
}

bool sequence_definition::equals(statement const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void sequence_definition::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "name"sv, name_);
    property(acceptor, "type"sv, type_);
    property(acceptor, "initial_value"sv, initial_value_);
    property(acceptor, "increment_value"sv, increment_value_);
    property(acceptor, "min_value"sv, min_value_);
    property(acceptor, "max_value"sv, max_value_);
    property(acceptor, "owner"sv, owner_);
    property(acceptor, "options"sv, options_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, sequence_definition const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::statement
