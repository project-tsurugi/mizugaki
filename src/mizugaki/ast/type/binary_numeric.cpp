#include <mizugaki/ast/type/binary_numeric.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

#include "utils.h"

namespace mizugaki::ast::type {

using node_kind_type = type::node_kind_type;
using type_kind_type = binary_numeric::type_kind_type;
using precision_type = binary_numeric::precision_type;


binary_numeric::binary_numeric(
        type_kind_type type_kind,
        std::optional<precision_type> precision,
        region_type region) :
    super { region },
    type_kind_ { type_kind },
    precision_ { precision }
{
    utils::validate_kind(tags, *type_kind);
}

binary_numeric::binary_numeric(::takatori::util::clone_tag_t, binary_numeric const& other) :
    binary_numeric {
            other.type_kind_,
            other.precision_,
            other.region(),
    }
{}

binary_numeric::binary_numeric(::takatori::util::clone_tag_t, binary_numeric&& other) :
    binary_numeric {
            other.type_kind_,
            other.precision_,
            other.region(),
    }
{}

binary_numeric* binary_numeric::clone() const& {
    return new binary_numeric(::takatori::util::clone_tag, *this); // NOLINT
}

binary_numeric* binary_numeric::clone() && {
    return new binary_numeric(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
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
    return precision_ == common::regioned { flexible_precision };
}

std::optional<precision_type>& binary_numeric::precision() noexcept {
    return precision_;
}

std::optional<precision_type> const& binary_numeric::precision() const noexcept {
    return precision_;
}

bool operator==(binary_numeric const& a, binary_numeric const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return true;
    }
    return eq(a.type_kind_, b.type_kind_)
        && eq(a.precision_, b.precision_);
}

bool operator!=(binary_numeric const& a, binary_numeric const& b) noexcept {
    return !(a == b);
}

bool binary_numeric::equals(type const& other) const noexcept {
    return tags.contains(other.node_kind())
            && *this == unsafe_downcast<binary_numeric>(other);
}

void binary_numeric::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "precision"sv, precision_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, binary_numeric const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::type
