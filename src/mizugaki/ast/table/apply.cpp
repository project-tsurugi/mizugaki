#include <mizugaki/ast/table/apply.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>
#include <mizugaki/ast/common/vector.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::table {

using ::takatori::util::clone_unique;

apply::apply(
        std::unique_ptr<table::expression> operand,
        std::optional<operator_kind_type> operator_kind,
        std::unique_ptr<name::name> name,
        std::vector<std::unique_ptr<scalar::expression>> arguments,
        correlation_type correlation,
        region_type region) noexcept:
    super { region },
    operand_ { std::move(operand) },
    operator_kind_ { operator_kind },
    name_ { std::move(name) },
    arguments_ { std::move(arguments) },
    correlation_ { std::move(correlation) }
{}

apply::apply(
        table::expression&& operand,
        name::name&& name,
        common::rvalue_list<scalar::expression> arguments,
        correlation_type correlation,
        std::optional<operator_kind_type> operator_kind,
        region_type region) noexcept:
    apply {
            clone_unique(std::move(operand)),
            operator_kind,
            clone_unique(std::move(name)),
            common::to_vector(arguments),
            std::move(correlation),
            region,
    }
{}

apply::apply(takatori::util::clone_tag_t, apply const& other):
    apply {
            clone_unique(other.operand_),
            other.operator_kind_,
            clone_unique(other.name_),
            common::clone_vector(other.arguments_),
            other.correlation_,
            other.region(),
    }
{}

apply::apply(takatori::util::clone_tag_t, apply&& other):
    apply {
            clone_unique(std::move(other.operand_)),
            other.operator_kind_,
            clone_unique(std::move(other.name_)),
            common::clone_vector(std::move(other.arguments_)),
            std::move(other.correlation_),
            other.region(),
    }
{}

apply* apply::clone() const & {
    return new apply(::takatori::util::clone_tag, *this); // NOLINT
}

apply* apply::clone() && {
    return new apply(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

expression::node_kind_type apply::node_kind() const noexcept {
    return tag;
}

std::unique_ptr<table::expression>& apply::operand() noexcept {
    return operand_;
}

std::unique_ptr<table::expression> const& apply::operand() const noexcept {
    return operand_;
}

std::optional<apply::operator_kind_type>& apply::operator_kind() noexcept {
    return operator_kind_;
}

std::optional<apply::operator_kind_type> const& apply::operator_kind() const noexcept {
    return operator_kind_;
}

std::unique_ptr<name::name>& apply::name() noexcept {
    return name_;
}

std::unique_ptr<name::name> const& apply::name() const noexcept {
    return name_;
}

std::vector<std::unique_ptr<scalar::expression>>& apply::arguments() noexcept {
    return arguments_;
}

std::vector<std::unique_ptr<scalar::expression>> const& apply::arguments() const noexcept {
    return arguments_;
}

apply::correlation_type& apply::correlation() noexcept {
    return correlation_;
}

apply::correlation_type const& apply::correlation() const noexcept {
    return correlation_;
}

bool apply::equals(expression const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void apply::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "operator_kind"sv, operator_kind_);
    property(acceptor, "name"sv, name_);
    property(acceptor, "arguments"sv, arguments_);
    property(acceptor, "correlation"sv, correlation_);
    region_property(acceptor, *this);
}

bool operator==(apply const& a, apply const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return true;
    }
    return eq(a.operator_kind_, b.operator_kind_)
            && eq(a.name_, b.name_)
            && eq(a.arguments_, b.arguments_)
            && eq(a.correlation_, b.correlation_);
}

bool operator!=(apply const& a, apply const& b) noexcept {
    return !(a == b);
}

std::ostream& operator<<(std::ostream& out, apply const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::table
