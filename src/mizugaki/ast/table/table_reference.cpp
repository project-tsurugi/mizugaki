#include <mizugaki/ast/table/table_reference.h>

#include <mizugaki/ast/common/optional.h>

#include <takatori/util/clonable.h>

namespace mizugaki::ast::table {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

using common::clone_optional;

table_reference::table_reference(
        unique_object_ptr<name::name> name,
        std::optional<correlation_type> correlation,
        bool_type is_only,
        region_type region) noexcept:
    super { region },
    name_ { std::move(name) },
    correlation_ { std::move(correlation) },
    is_only_ { is_only }
{}

table_reference::table_reference(table_reference const& other, object_creator creator) :
    table_reference {
            clone_unique(other.name_, creator),
            clone_optional(other.correlation_, creator),
            other.is_only_,
            other.region(),
    }
{}

table_reference::table_reference(table_reference&& other, object_creator creator) :
    table_reference {
            clone_unique(std::move(other.name_), creator),
            clone_optional(std::move(other.correlation_), creator),
            other.is_only_,
            other.region(),
    }
{}

table_reference* table_reference::clone(object_creator creator) const& {
    return creator.create_object<table_reference>(*this, creator);
}

table_reference* table_reference::clone(object_creator creator) && {
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

std::optional<table_reference::correlation_type>& table_reference::correlation() noexcept {
    return correlation_;
}

std::optional<table_reference::correlation_type> const& table_reference::correlation() const noexcept {
    return correlation_;
}

table_reference::bool_type& table_reference::is_only() noexcept {
    return is_only_;
}

table_reference::bool_type const& table_reference::is_only() const noexcept {
    return is_only_;
}

} // namespace mizugaki::ast::table