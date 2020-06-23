#include <mizugaki/ast/type/collection.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::type {

using node_kind_type = type::node_kind_type;
using length_type = collection::length_type;

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

collection::collection(
        unique_object_ptr<type> element,
        std::optional<length_type> length,
        region_type region) noexcept :
    super { region },
    element_ { std::move(element) },
    length_ { std::move(length) }
{}

collection::collection(
        type&& element,
        std::optional<length_type> length,
        region_type region) :
    collection {
            clone_unique(std::move(element)),
            std::move(length),
            region,
    }
{}

collection::collection(collection const& other, object_creator creator) :
    collection {
            clone_unique(other.element_, creator),
            other.length_,
            other.region(),
    }
{}

collection::collection(collection&& other, object_creator creator) :
    collection {
            clone_unique(std::move(other.element_), creator),
            std::move(other.length_),
            other.region(),
    }
{}

collection* collection::clone(object_creator creator) const& {
    return creator.create_object<collection>(*this, creator);
}

collection* collection::clone(object_creator creator) && {
    return creator.create_object<collection>(std::move(*this), creator);
}

node_kind_type collection::node_kind() const noexcept {
    return tag;
}

unique_object_ptr<type>& collection::element() noexcept {
    return element_;
}

unique_object_ptr<type> const& collection::element() const noexcept {
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
