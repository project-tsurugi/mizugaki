#include <mizugaki/ast/name/qualified.h>

#include <takatori/util/clonable.h>

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
    qualified(
            clone_unique(other.qualifier_, creator),
            clone_unique(other.last_, creator),
            other.region())
{}

qualified::qualified(qualified&& other, object_creator creator) :
    qualified(
            clone_unique(std::move(other.qualifier_), creator),
            clone_unique(std::move(other.last_), creator),
            other.region())
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

simple& qualified::last() noexcept {
    return *last_;
}

simple const& qualified::last() const noexcept {
    return *last_;
}

unique_object_ptr<simple>& qualified::mutable_last() noexcept {
    return last_;
}

name& qualified::qualifier() noexcept {
    return *qualifier_;
}

name const& qualified::qualifier() const noexcept {
    return *qualifier_;
}

optional_ptr<name> qualified::optional_qualifier() noexcept {
    return optional_ptr { qualifier_.get() };
}

optional_ptr<name const> qualified::optional_qualifier() const noexcept {
    return optional_ptr { qualifier_.get() };
}

::takatori::util::unique_object_ptr<name>& qualified::mutable_qualifier() noexcept {
    return qualifier_;
}

} // namespace mizugaki::ast::name
