#include <mizugaki/ast/scalar/field_reference.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::scalar {

using operator_kind_type = field_reference::operator_kind_type;

using ::takatori::util::clone_unique;

field_reference::field_reference(
        operand_type value,
        operator_kind_type operator_kind,
        std::unique_ptr<name::simple> name,
        region_type region) noexcept:
    super { region },
    value_ { std::move(value) },
    operator_kind_ { operator_kind },
    name_ { std::move(name) }
{}

field_reference::field_reference(
        expression&& value,
        name::simple&& name,
        operator_kind_type operator_kind,
        region_type region) :
    field_reference {
            clone_unique(std::move(value)),
            operator_kind,
            clone_unique(std::move(name)),
            region,
    }
{}

field_reference::field_reference(::takatori::util::clone_tag_t, field_reference const& other) :
    field_reference {
            clone_unique(other.value_),
            other.operator_kind_,
            clone_unique(other.name_),
            other.region(),
    }
{}

field_reference::field_reference(::takatori::util::clone_tag_t, field_reference&& other) :
    field_reference {
            clone_unique(std::move(other.value_)),
            other.operator_kind_,
            clone_unique(std::move(other.name_)),
            other.region(),
    }
{}

field_reference* field_reference::clone() const& {
    return new field_reference(::takatori::util::clone_tag, *this); // NOLINT
}

field_reference* field_reference::clone() && {
    return new field_reference(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

expression::node_kind_type field_reference::node_kind() const noexcept {
    return tag;
}

operator_kind_type& field_reference::operator_kind() noexcept {
    return operator_kind_;
}

operator_kind_type const& field_reference::operator_kind() const noexcept {
    return operator_kind_;
}

expression::operand_type& field_reference::value() noexcept {
    return value_;
}

expression::operand_type const& field_reference::value() const noexcept {
    return value_;
}

std::unique_ptr<name::simple>& field_reference::name() noexcept {
    return name_;
}

std::unique_ptr<name::simple> const& field_reference::name() const noexcept {
    return name_;
}

bool operator==(field_reference const& a, field_reference const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return false;
    }
    return eq(a.operator_kind_, b.operator_kind_)
            && eq(a.value_, b.value_)
            && eq(a.name_, b.name_);
}

bool operator!=(field_reference const& a, field_reference const& b) noexcept {
    return !(a == b);
}

bool field_reference::equals(expression const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void field_reference::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "value"sv, value_);
    property(acceptor, "operator_kind"sv, operator_kind_);
    property(acceptor, "name"sv, name_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, field_reference const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::scalar
