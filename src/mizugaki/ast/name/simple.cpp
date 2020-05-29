#include <mizugaki/ast/name/simple.h>

namespace mizugaki::ast::name {

using identifier_type = simple::identifier_type;

using ::takatori::util::object_creator;
using ::takatori::util::optional_ptr;

simple::simple(identifier_type identifier, region_type region) noexcept :
    super { region },
    identifier_ { std::move(identifier) }
{}

simple::simple(simple const& other, object_creator creator) :
    simple(
        identifier_type { other.identifier_, creator.allocator() },
            other.region())
{}

simple::simple(simple&& other, object_creator creator) :
    simple(
            identifier_type { std::move(other.identifier_), creator.allocator() },
            other.region())
{}

simple* simple::clone(object_creator creator) const& {
    return creator.create_object<simple>(*this, creator);
}

simple* simple::clone(object_creator creator) && {
    return creator.create_object<simple>(std::move(*this), creator);
}

name::node_kind_type simple::node_kind() const noexcept {
    return tag;
}

simple& simple::last() noexcept {
    return *this;
}

simple const& simple::last() const noexcept {
    return *this;
}

optional_ptr<name> simple::optional_qualifier() noexcept {
    return {};
}

optional_ptr<name const> simple::optional_qualifier() const noexcept {
    return {};
}

identifier_type& simple::identifier() noexcept {
    return identifier_;
}

identifier_type const& simple::identifier() const noexcept {
    return identifier_;
}

} // namespace mizugaki::ast::name
