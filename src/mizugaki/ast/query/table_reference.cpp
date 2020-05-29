#include <mizugaki/ast/query/table_reference.h>

#include <takatori/util/clonable.h>

namespace mizugaki::ast::query {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

table_reference::table_reference(
        unique_object_ptr<name::name> name,
        region_type region) noexcept :
    super { region },
    name_ { std::move(name) }
{}

table_reference::table_reference(table_reference const& other, object_creator creator) :
    table_reference {
            clone_unique(other.name_, creator),
            other.region(),
    }
{}

table_reference::table_reference(table_reference&& other, object_creator creator) :
    table_reference {
            clone_unique(std::move(other.name_), creator),
            other.region(),
    }
{}

table_reference* table_reference::clone(object_creator creator) const& {
    return creator.create_object<table_reference>(*this, creator);
}

table_reference* table_reference::clone(object_creator creator)&& {
    return creator.create_object<table_reference>(std::move(*this), creator);
}

expression::node_kind_type table_reference::node_kind() const noexcept {
    return tag;
}

unique_object_ptr<name::name>& table_reference::name() noexcept {
    return name_;
}

unique_object_ptr<name::name> const& table_reference::name() const noexcept {
    return name_;
}

} // namespace mizugaki::ast::query
