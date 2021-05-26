#include <mizugaki/ast/type/collection.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::type {

using node_kind_type = type::node_kind_type;
using length_type = collection::length_type;

using ::takatori::util::clone_unique;

collection::collection(
        std::unique_ptr<type> element,
        std::optional<length_type> length,
        region_type region) noexcept :
    super { region },
    element_ { std::move(element) },
    length_ { length }
{}

collection::collection(
        type&& element,
        std::optional<length_type> length,
        region_type region) :
    collection {
            clone_unique(std::move(element)),
            length,
            region,
    }
{}

collection::collection(::takatori::util::clone_tag_t, collection const& other) :
    collection {
            clone_unique(other.element_),
            other.length_,
            other.region(),
    }
{}

collection::collection(::takatori::util::clone_tag_t, collection&& other) :
    collection {
            clone_unique(std::move(other.element_)),
            other.length_,
            other.region(),
    }
{}

collection* collection::clone() const& {
    return new collection(::takatori::util::clone_tag, *this); // NOLINT
}

collection* collection::clone() && {
    return new collection(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

node_kind_type collection::node_kind() const noexcept {
    return tag;
}

std::unique_ptr<type>& collection::element() noexcept {
    return element_;
}

std::unique_ptr<type> const& collection::element() const noexcept {
    return element_;
}

bool collection::is_flexible_length() const noexcept {
    return length_ == common::regioned { flexible_length };
}

std::optional<length_type>& collection::length() noexcept {
    return length_;
}

std::optional<length_type> const& collection::length() const noexcept {
    return length_;
}

bool operator==(collection const& a, collection const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return true;
    }
    return eq(a.element_, b.element_)
            && eq(a.length_, b.length_);
}

bool operator!=(collection const& a, collection const& b) noexcept {
    return !(a == b);
}

bool collection::equals(ast::type::type const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<collection>(other);
}

void collection::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "element"sv, element_);
    property(acceptor, "length"sv, length_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, collection const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::type
