#include <mizugaki/ast/type/decimal.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

#include "utils.h"

namespace mizugaki::ast::type {

using node_kind_type = type::node_kind_type;
using type_kind_type = decimal::type_kind_type;
using precision_type = decimal::precision_type;
using scale_type = decimal::scale_type;


decimal::decimal(
        type_kind_type type_kind,
        std::optional<precision_type> precision,
        std::optional<scale_type> scale,
        region_type region) :
    super { region },
    type_kind_ { type_kind },
    precision_ { precision },
    scale_ { scale }
{
    utils::validate_kind(tags, *type_kind);
}

decimal::decimal(::takatori::util::clone_tag_t, decimal const& other) :
    decimal {
            other.type_kind_,
            other.precision_,
            other.scale_,
            other.region(),
    }
{}

decimal::decimal(::takatori::util::clone_tag_t, decimal&& other) :
    decimal {
            other.type_kind_,
            other.precision_,
            other.scale_,
            other.region(),
    }
{}

decimal* decimal::clone() const& {
    return new decimal(::takatori::util::clone_tag, *this); // NOLINT
}

decimal* decimal::clone() && {
    return new decimal(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

node_kind_type decimal::node_kind() const noexcept {
    return *type_kind_;
}

type_kind_type& decimal::type_kind() noexcept {
    return type_kind_;
}

type_kind_type const& decimal::type_kind() const noexcept {
    return type_kind_;
}

bool decimal::is_flexible_precision() const noexcept {
    return precision_ == flexible_precision;
}

bool decimal::is_flexible_scale() const noexcept {
    return scale_ == flexible_scale;
}

std::optional<precision_type>& decimal::precision() noexcept {
    return precision_;
}

std::optional<precision_type> const& decimal::precision() const noexcept {
    return precision_;
}

std::optional<scale_type>& decimal::scale() noexcept {
    return scale_;
}

std::optional<scale_type> const& decimal::scale() const noexcept {
    return scale_;
}

bool operator==(decimal const& a, decimal const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return true;
    }
    return eq(a.type_kind_, b.type_kind_)
            && eq(a.precision_, b.precision_)
            && eq(a.scale_, b.scale_);
}

bool operator!=(decimal const& a, decimal const& b) noexcept {
    return !(a == b);
}

bool decimal::equals(type const& other) const noexcept {
    return tags.contains(other.node_kind())
            && *this == unsafe_downcast<decimal>(other);
}

void decimal::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "precision"sv, precision_);
    property(acceptor, "scale"sv, scale_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, decimal const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::type
