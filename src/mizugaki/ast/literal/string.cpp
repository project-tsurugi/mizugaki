#include <mizugaki/ast/literal/string.h> // NOLINT

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

#include "utils.h"

namespace mizugaki::ast::literal {

using node_kind_type = literal::node_kind_type;
using value_kind_type = string::value_kind_type;
using value_type = string::value_type;
using concatenations_type = string::concatenations_type;


string::string(
        value_kind_type value_kind,
        value_type value,
        concatenations_type concatenations,
        region_type region) :
    super { region },
    value_kind_ { value_kind },
    value_ { std::move(value) },
    concatenations_ { std::move(concatenations) }
{
    utils::validate_kind(tags, *value_kind);
}

string::string(::takatori::util::clone_tag_t, string const& other) :
    string {
            other.value_kind_,
            value_type { other.value_ },
            concatenations_type { other.concatenations_ },
            other.region(),
    }
{}

string::string(::takatori::util::clone_tag_t, string&& other) :
    string{
            other.value_kind_,
            value_type { std::move(other.value_) },
            concatenations_type { std::move(other.concatenations_) },
            other.region(),
    }
{}

string* string::clone() const& {
    return new string(::takatori::util::clone_tag, *this); // NOLINT
}

string* string::clone() && {
    return new string(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

node_kind_type string::node_kind() const noexcept {
    return *value_kind_;
}

value_kind_type& string::value_kind() noexcept {
    return value_kind_;
}

value_kind_type const& string::value_kind() const noexcept {
    return value_kind_;
}

value_type& string::value() noexcept {
    return value_;
}

value_type const& string::value() const noexcept {
    return value_;
}

concatenations_type& string::concatenations() noexcept {
    return concatenations_;
}

concatenations_type const& string::concatenations() const noexcept {
    return concatenations_;
}

bool operator==(string const& a, string const& b) noexcept {
    return eq(a.value_kind_, b.value_kind_)
            && eq(a.value_, b.value_)
            && eq(a.concatenations_, b.concatenations_);
}

bool operator!=(string const& a, string const& b) noexcept {
    return !(a == b);
}

bool string::equals(literal const& other) const noexcept {
    return tags.contains(other.node_kind())
            && *this == unsafe_downcast<string>(other);
}

void string::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "value"sv, value_);
    property(acceptor, "concatenations"sv, concatenations_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, string const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::literal
