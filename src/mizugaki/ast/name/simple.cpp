#include <mizugaki/ast/name/simple.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::name {

using ::takatori::util::optional_ptr;

simple::simple(identifier_type identifier, region_type region) noexcept :
    super { region },
    identifier_ { std::move(identifier) }
{}

simple::simple(::takatori::util::clone_tag_t, simple const& other) :
    simple {
            { other.identifier_ },
            other.region(),
    }
{}

simple::simple(::takatori::util::clone_tag_t, simple&& other) :
    simple {
            { std::move(other.identifier_) },
            other.region(),
    }
{}

simple* simple::clone() const& {
    return new simple(::takatori::util::clone_tag, *this); // NOLINT
}

simple* simple::clone() && {
    return new simple(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

name::node_kind_type simple::node_kind() const noexcept {
    return tag;
}

name::identifier_type const& simple::last_identifier() const noexcept {
    return identifier();
}

optional_ptr<name const> simple::optional_qualifier() const noexcept {
    return {};
}

name::identifier_type& simple::identifier() noexcept {
    return identifier_;
}

name::identifier_type const& simple::identifier() const noexcept {
    return identifier_;
}

bool operator==(simple const& a, simple const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return true;
    }
    return eq(a.identifier_, b.identifier_);
}

bool operator!=(simple const& a, simple const& b) noexcept {
    return !(a == b);
}

bool simple::equals(name const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void simple::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "identifier"sv, identifier_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, simple const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::name
