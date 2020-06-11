#include <mizugaki/ast/query/select_asterisk.h>

#include <takatori/util/clonable.h>

namespace mizugaki::ast::query {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

select_asterisk::select_asterisk(
        unique_object_ptr<name::name> qualifier,
        region_type region) noexcept :
    super { region },
    qualifier_ { std::move(qualifier) }
{}

select_asterisk::select_asterisk(select_asterisk const& other, object_creator creator) :
    select_asterisk {
            clone_unique(other.qualifier_, creator),
            other.region(),
    }
{}

select_asterisk::select_asterisk(select_asterisk&& other, object_creator creator) :
    select_asterisk {
            clone_unique(std::move(other.qualifier_), creator),
            other.region(),
    }
{}

select_asterisk* select_asterisk::clone(::takatori::util::object_creator creator) const& {
    return creator.create_object<select_asterisk>(*this, creator);
}

select_asterisk* select_asterisk::clone(::takatori::util::object_creator creator)&& {
    return creator.create_object<select_asterisk>(std::move(*this), creator);
}

select_element::node_kind_type select_asterisk::node_kind() const noexcept {
    return tag;
}

unique_object_ptr<name::name>& select_asterisk::qualifier() noexcept {
    return qualifier_;
}

unique_object_ptr<name::name> const& select_asterisk::qualifier() const noexcept {
    return qualifier_;
}

} // namespace mizugaki::ast::query