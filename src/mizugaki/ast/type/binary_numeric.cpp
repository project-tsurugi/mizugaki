#include <mizugaki/ast/type/binary_numeric.h>

#include "utils.h"

namespace mizugaki::ast::type {

using node_kind_type = type::node_kind_type;
using type_kind_type = binary_numeric::type_kind_type;
using precision_type = binary_numeric::precision_type;

using ::takatori::util::object_creator;

binary_numeric::binary_numeric(
        type_kind_type type_kind,
        std::optional<precision_type> precision,
        region_type region) :
    super { region },
    type_kind_ { type_kind },
    precision_ { std::move(precision) }
{
    utils::validate_kind(tags, *type_kind);
}

binary_numeric::binary_numeric(binary_numeric const& other, object_creator) :
    binary_numeric {
            other.type_kind_,
            other.precision_,
            other.region(),
    }
{}

binary_numeric::binary_numeric(binary_numeric&& other, object_creator) :
    binary_numeric {
            other.type_kind_,
            std::move(other.precision_),
            other.region(),
    }
{}

binary_numeric* binary_numeric::clone(object_creator creator) const& {
    return creator.create_object<binary_numeric>(*this, creator);
}

binary_numeric* binary_numeric::clone(object_creator creator) && {
    return creator.create_object<binary_numeric>(std::move(*this), creator);
}

node_kind_type binary_numeric::node_kind() const noexcept {
    return *type_kind_;
}

type_kind_type& binary_numeric::type_kind() noexcept {
    return type_kind_;
}

type_kind_type const& binary_numeric::type_kind() const noexcept {
    return type_kind_;
}

bool binary_numeric::is_flexible_precision() const noexcept {
    return precision_ == flexible_precision;
}

std::optional<precision_type>& binary_numeric::precision() noexcept {
    return precision_;
}

std::optional<precision_type> const& binary_numeric::precision() const noexcept {
    return precision_;
}

} // namespace mizugaki::ast::type
