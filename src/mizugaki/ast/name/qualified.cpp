#include <mizugaki/ast/name/qualified.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>
#include <mizugaki/ast/name/simple.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::name {

using ::takatori::util::clone_unique;
using ::takatori::util::optional_ptr;

qualified::qualified(
        std::unique_ptr<name> qualifier,
        std::unique_ptr<simple> last,
        region_type region) noexcept :
    super { region },
    qualifier_ { std::move(qualifier) },
    last_ { std::move(last) }
{}

qualified::qualified(
        name&& qualifier,
        simple&& last,
        region_type region) :
    qualified {
            clone_unique(std::move(qualifier)),
            clone_unique(std::move(last)),
            region,
    }
{}

qualified::qualified(::takatori::util::clone_tag_t, qualified const& other) :
    qualified{
            clone_unique(other.qualifier_),
            clone_unique(other.last_),
            other.region(),
    }
{}

qualified::qualified(::takatori::util::clone_tag_t, qualified&& other) :
    qualified{
            clone_unique(std::move(other.qualifier_)),
            clone_unique(std::move(other.last_)),
            other.region(),
    }
{}

qualified* qualified::clone() const& {
    return new qualified(::takatori::util::clone_tag, *this); // NOLINT
}

qualified* qualified::clone() && {
    return new qualified(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

name::node_kind_type qualified::node_kind() const noexcept {
    return tag;
}

simple const& qualified::last_name() const noexcept {
    return *last_;
}

optional_ptr<name const> qualified::optional_qualifier() const noexcept {
    return takatori::util::optional_ptr { qualifier_.get() };
}

std::unique_ptr<name>& qualified::qualifier() noexcept {
    return qualifier_;
}

std::unique_ptr<name> const& qualified::qualifier() const noexcept {
    return qualifier_;
}

std::unique_ptr<simple>& qualified::last() noexcept {
    return last_;
}

std::unique_ptr<simple> const& qualified::last() const noexcept {
    return last_;
}

bool operator==(qualified const& a, qualified const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return true;
    }
    return eq(a.last_, b.last_)
        && eq(a.qualifier_, b.qualifier_);
}

bool operator!=(qualified const& a, qualified const& b) noexcept {
    return !(a == b);
}

bool qualified::equals(name const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void qualified::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "last"sv, last_);
    property(acceptor, "qualifier"sv, qualifier_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, qualified const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::name
