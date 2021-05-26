#include <mizugaki/ast/literal/numeric.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

#include "utils.h"

namespace mizugaki::ast::literal {

using node_kind_type = literal::node_kind_type;
using sign_type = numeric::sign_type;
using value_type = numeric::value_type;

numeric::numeric(
        value_kind_type value_kind,
        std::optional<sign_type> sign,
        value_type unsigned_value,
        region_type region) :
    super { region },
    value_kind_ { value_kind },
    sign_ { sign },
    unsigned_value_ { std::move(unsigned_value) }
{
    utils::validate_kind(tags, value_kind);
}

numeric::numeric(::takatori::util::clone_tag_t, numeric const& other) :
    numeric {
            other.value_kind_,
            other.sign_,
            value_type{other.unsigned_value_},
            other.region(),
    }
{}

numeric::numeric(::takatori::util::clone_tag_t, numeric&& other) :
    numeric {
            other.value_kind_,
            other.sign_,
            value_type { std::move(other.unsigned_value_) },
            other.region(),
    }
{}

numeric* numeric::clone() const& {
    return new numeric(::takatori::util::clone_tag, *this); // NOLINT
}

numeric* numeric::clone() && {
    return new numeric(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

node_kind_type numeric::node_kind() const noexcept {
    return value_kind_;
}

numeric::value_kind_type& numeric::value_kind() noexcept {
    return value_kind_;
}

numeric::value_kind_type const& numeric::value_kind() const noexcept {
    return value_kind_;
}

std::optional<sign_type>& numeric::sign() noexcept {
    return sign_;
}

std::optional<sign_type> const& numeric::sign() const noexcept {
    return sign_;
}

value_type& numeric::unsigned_value() noexcept {
    return unsigned_value_;
}

value_type const& numeric::unsigned_value() const noexcept {
    return unsigned_value_;
}

bool operator==(numeric const& a, numeric const& b) noexcept {
    return eq(a.value_kind_, b.value_kind_)
            && eq(a.sign_, b.sign_)
            && eq(a.unsigned_value_, b.unsigned_value_);
}

bool operator!=(numeric const& a, numeric const& b) noexcept {
    return !(a == b);
}

bool numeric::equals(literal const& other) const noexcept {
    return tags.contains(other.node_kind())
            && *this == unsafe_downcast<numeric>(other);
}

void numeric::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "sign"sv, sign_);
    property(acceptor, "unsigned_value"sv, unsigned_value_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, numeric const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::literal
