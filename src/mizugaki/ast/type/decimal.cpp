#include <mizugaki/ast/type/decimal.h>

#include "utils.h"

namespace mizugaki::ast::type {

using node_kind_type = type::node_kind_type;
using type_kind_type = decimal::type_kind_type;
using precision_type = decimal::precision_type;
using scale_type = decimal::scale_type;

using ::takatori::util::object_creator;

decimal::decimal(
        type_kind_type type_kind,
        std::optional<precision_type> precision,
        std::optional<scale_type> scale,
        region_type region) :
    super { region },
    type_kind_ { type_kind },
    precision_ { std::move(precision) },
    scale_ { std::move(scale) }
{
    utils::validate_kind(tags, *type_kind);
}

decimal::decimal(decimal const& other, object_creator) :
    decimal {
            other.type_kind_,
            other.precision_,
            other.scale_,
            other.region(),
    }
{}

decimal::decimal(decimal&& other, object_creator) :
    decimal {
            other.type_kind_,
            std::move(other.precision_),
            std::move(other.scale_),
            other.region(),
    }
{}

decimal* decimal::clone(object_creator creator) const& {
    return creator.create_object<decimal>(*this, creator);
}

decimal* decimal::clone(object_creator creator) && {
    return creator.create_object<decimal>(std::move(*this), creator);
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

} // namespace mizugaki::ast::type
