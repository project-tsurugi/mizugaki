#include <mizugaki/ast/query/select_asterisk.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::query {

using ::takatori::util::clone_unique;

select_asterisk::select_asterisk(
        std::unique_ptr<scalar::expression> qualifier,
        region_type region) noexcept :
    super { region },
    qualifier_ { std::move(qualifier) }
{}

select_asterisk::select_asterisk(
        scalar::expression&& qualifier,
        region_type region) :
    select_asterisk {
            clone_unique(std::move(qualifier)),
            region,
    }
{}

select_asterisk::select_asterisk(::takatori::util::clone_tag_t, select_asterisk const& other) :
    select_asterisk {
            clone_unique(other.qualifier_),
            other.region(),
    }
{}

select_asterisk::select_asterisk(::takatori::util::clone_tag_t, select_asterisk&& other) :
    select_asterisk {
            clone_unique(std::move(other.qualifier_)),
            other.region(),
    }
{}

select_asterisk* select_asterisk::clone() const& {
    return new select_asterisk(::takatori::util::clone_tag, *this); // NOLINT
}

select_asterisk* select_asterisk::clone()&& {
    return new select_asterisk(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

select_element::node_kind_type select_asterisk::node_kind() const noexcept {
    return tag;
}

std::unique_ptr<scalar::expression>& select_asterisk::qualifier() noexcept {
    return qualifier_;
}

std::unique_ptr<scalar::expression> const& select_asterisk::qualifier() const noexcept {
    return qualifier_;
}

bool operator==(select_asterisk const& a, select_asterisk const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return false;
    }
    return eq(a.qualifier_, b.qualifier_);
}

bool operator!=(select_asterisk const& a, select_asterisk const& b) noexcept {
    return !(a == b);
}

bool select_asterisk::equals(select_element const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void select_asterisk::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "qualifier"sv, qualifier_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, select_asterisk const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::query
