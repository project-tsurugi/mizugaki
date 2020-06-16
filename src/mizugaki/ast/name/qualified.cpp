#include <mizugaki/ast/name/qualified.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/compare_utils.h>
#include <mizugaki/ast/name/simple.h>

namespace mizugaki::ast::name {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::optional_ptr;
using ::takatori::util::unique_object_ptr;

qualified::qualified(
        unique_object_ptr<name> qualifier,
        unique_object_ptr<simple> last,
        region_type region) noexcept :
    super { region },
    qualifier_ { std::move(qualifier) },
    last_ { std::move(last) }
{}

qualified::qualified(qualified const& other, object_creator creator) :
    qualified{
            clone_unique(other.qualifier_, creator),
            clone_unique(other.last_, creator),
            other.region(),
    }
{}

qualified::qualified(qualified&& other, object_creator creator) :
    qualified{
            clone_unique(std::move(other.qualifier_), creator),
            clone_unique(std::move(other.last_), creator),
            other.region(),
    }
{}

qualified* qualified::clone(object_creator creator) const& {
    return creator.create_object<qualified>(*this, creator);
}

qualified* qualified::clone(object_creator creator) && {
    return creator.create_object<qualified>(std::move(*this), creator);
}

name::node_kind_type qualified::node_kind() const noexcept {
    return tag;
}

name::identifier_type const& qualified::last_identifier() const noexcept {
    return last()->identifier();
}

optional_ptr<name const> qualified::optional_qualifier() const noexcept {
    return takatori::util::optional_ptr<name const>();
}

unique_object_ptr<name>& qualified::qualifier() noexcept {
    return qualifier_;
}

unique_object_ptr<name> const& qualified::qualifier() const noexcept {
    return qualifier_;
}

unique_object_ptr<simple>& qualified::last() noexcept {
    return last_;
}

unique_object_ptr<simple> const& qualified::last() const noexcept {
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
            && *this == unsafe_downcast<simple>(other);
}

} // namespace mizugaki::ast::name
