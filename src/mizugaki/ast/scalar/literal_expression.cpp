#include <mizugaki/ast/scalar/literal_expression.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::scalar {

using ::takatori::util::clone_unique;

literal_expression::literal_expression(
        std::unique_ptr<literal::literal> value,
        region_type region) noexcept:
    super { region },
    value_ { std::move(value) }
{}

literal_expression::literal_expression(
        literal::literal&& value,
        region_type region) :
    literal_expression {
            clone_unique(std::move(value)),
            region,
    }
{}

literal_expression::literal_expression(::takatori::util::clone_tag_t, literal_expression const& other) :
    literal_expression {
            clone_unique(other.value_),
            other.region(),
    }
{}

literal_expression::literal_expression(::takatori::util::clone_tag_t, literal_expression&& other) :
    literal_expression {
            clone_unique(std::move(other.value_)),
            other.region(),
    }
{}

literal_expression* literal_expression::clone() const& {
    return new literal_expression(::takatori::util::clone_tag, *this); // NOLINT
}

literal_expression* literal_expression::clone() && {
    return new literal_expression(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

expression::node_kind_type literal_expression::node_kind() const noexcept {
    return tag;
}

std::unique_ptr<literal::literal>& literal_expression::value() noexcept {
    return value_;
}

std::unique_ptr<literal::literal> const& literal_expression::value() const noexcept {
    return value_;
}

bool operator==(literal_expression const& a, literal_expression const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return false;
    }
    return eq(a.value_, b.value_);
}

bool operator!=(literal_expression const& a, literal_expression const& b) noexcept {
    return !(a == b);
}

bool literal_expression::equals(expression const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void literal_expression::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "value"sv, value_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, literal_expression const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::scalar
