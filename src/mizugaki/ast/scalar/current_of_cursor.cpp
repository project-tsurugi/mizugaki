#include <mizugaki/ast/scalar/current_of_cursor.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::scalar {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

current_of_cursor::current_of_cursor(
        unique_object_ptr<name::name> name,
        region_type region) noexcept:
    super { region },
    name_ { std::move(name) }
{}

current_of_cursor::current_of_cursor(
        name::name&& name,
        region_type region) :
    current_of_cursor {
            clone_unique(std::move(name)),
            region,
    }
{}

current_of_cursor::current_of_cursor(current_of_cursor const& other, object_creator creator) :
    current_of_cursor {
            clone_unique(other.name_, creator),
            other.region(),
    }
{}

current_of_cursor::current_of_cursor(current_of_cursor&& other, object_creator creator) :
    current_of_cursor {
            clone_unique(std::move(other.name_), creator),
            other.region(),
    }
{}

current_of_cursor* current_of_cursor::clone(object_creator creator) const& {
    return creator.create_object<current_of_cursor>(*this, creator);
}

current_of_cursor* current_of_cursor::clone(object_creator creator) && {
    return creator.create_object<current_of_cursor>(std::move(*this), creator);
}

expression::node_kind_type current_of_cursor::node_kind() const noexcept {
    return tag;
}

unique_object_ptr<name::name>& current_of_cursor::name() noexcept {
    return name_;
}

unique_object_ptr<name::name> const& current_of_cursor::name() const noexcept {
    return name_;
}

bool operator==(current_of_cursor const& a, current_of_cursor const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return false;
    }
    return eq(a.name_, b.name_);
}

bool operator!=(current_of_cursor const& a, current_of_cursor const& b) noexcept {
    return !(a == b);
}

bool current_of_cursor::equals(expression const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void current_of_cursor::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "name"sv, name_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, current_of_cursor const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::scalar
