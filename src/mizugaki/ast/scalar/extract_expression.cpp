#include <mizugaki/ast/scalar/extract_expression.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::scalar {

using ::takatori::util::clone_unique;

extract_expression::extract_expression(
        field_type field,
        std::optional<size_type> subsecond_digits,
        operand_type operand,
        region_type region) noexcept :
    super { region },
    field_ { field },
    subsecond_digits_ { subsecond_digits },
    operand_ { std::move(operand) }
{}

extract_expression::extract_expression(
        field_type field,
        operand_type operand,
        region_type region) noexcept :
    extract_expression {
            field,
            std::nullopt,
            std::move(operand),
            region,
    }
{}

extract_expression::extract_expression(
        field_type field,
        expression&& operand,
        region_type region) :
    extract_expression {
            field,
            std::nullopt,
            clone_unique(std::move(operand)),
            region,
    }
{}

extract_expression::extract_expression(
        field_type field,
        size_type subsecond_digits,
        expression&& operand,
        region_type region) :
    extract_expression {
            field,
            subsecond_digits,
            clone_unique(std::move(operand)),
            region,
    }
{}

extract_expression::extract_expression(::takatori::util::clone_tag_t, extract_expression const& other) :
    extract_expression {
            other.field_,
            other.subsecond_digits_,
            clone_unique(other.operand_),
            other.region(),
    }
{}

extract_expression::extract_expression(::takatori::util::clone_tag_t, extract_expression&& other) :
    extract_expression {
            other.field_,
            other.subsecond_digits_,
            clone_unique(std::move(other.operand_)),
            other.region(),
    }
{}

extract_expression* extract_expression::clone() const& {
    return new extract_expression(::takatori::util::clone_tag, *this); // NOLINT
}

extract_expression* extract_expression::clone() && {
    return new extract_expression(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

expression::node_kind_type extract_expression::node_kind() const noexcept {
    return tag;
}

extract_expression::field_type& extract_expression::field() noexcept {
    return field_;
}

extract_expression::field_type const& extract_expression::field() const noexcept {
    return field_;
}

std::optional<extract_expression::size_type>& extract_expression::subsecond_digits() noexcept {
    return subsecond_digits_;
}

std::optional<extract_expression::size_type> const& extract_expression::subsecond_digits() const noexcept {
    return subsecond_digits_;
}

expression::operand_type& extract_expression::operand() noexcept {
    return operand_;
}

expression::operand_type const& extract_expression::operand() const noexcept {
    return operand_;
}

bool operator==(extract_expression const& a, extract_expression const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return false;
    }
    return eq(a.field_, b.field_)
        && eq(a.subsecond_digits_, b.subsecond_digits_)
        && eq(a.operand_, b.operand_);
}

bool operator!=(extract_expression const& a, extract_expression const& b) noexcept {
    return !(a == b);
}

bool extract_expression::equals(expression const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void extract_expression::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "field"sv, field_);
    property(acceptor, "subsecond_digits"sv, subsecond_digits_);
    property(acceptor, "operand"sv, operand_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, extract_expression const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::scalar
