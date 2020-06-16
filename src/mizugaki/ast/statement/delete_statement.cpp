#include <mizugaki/ast/statement/delete_statement.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::statement {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

delete_statement::delete_statement(
        unique_object_ptr<name::name> table_name,
        unique_object_ptr<scalar::expression> condition,
        region_type region) noexcept :
    super { region },
    table_name_ { std::move(table_name) },
    condition_ { std::move(condition) }
{}

delete_statement::delete_statement(delete_statement const& other, object_creator creator) :
    delete_statement {
            clone_unique(other.table_name_, creator),
            clone_unique(other.condition_, creator),
            other.region(),
    }
{}

delete_statement::delete_statement(delete_statement&& other, object_creator creator) :
    delete_statement {
            clone_unique(std::move(other.table_name_), creator),
            clone_unique(std::move(other.condition_), creator),
            other.region(),
    }
{}

delete_statement* delete_statement::clone(object_creator creator) const& {
    return creator.create_object<delete_statement>(*this, creator);
}

delete_statement* delete_statement::clone(object_creator creator) && {
    return creator.create_object<delete_statement>(std::move(*this), creator);
}

statement::node_kind_type delete_statement::node_kind() const noexcept {
    return tag;
}

unique_object_ptr<name::name>& delete_statement::table_name() noexcept {
    return table_name_;
}

unique_object_ptr<name::name> const& delete_statement::table_name() const noexcept {
    return table_name_;
}

unique_object_ptr<scalar::expression>& delete_statement::condition() noexcept {
    return condition_;
}

unique_object_ptr<scalar::expression> const& delete_statement::condition() const noexcept {
    return condition_;
}

bool operator==(delete_statement const& a, delete_statement const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return true;
    }
    return eq(a.table_name_, b.table_name_)
        && eq(a.condition_, b.condition_);
}

bool operator!=(delete_statement const& a, delete_statement const& b) noexcept {
    return !(a == b);
}

bool delete_statement::equals(statement const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<delete_statement>(other);
}

} // namespace mizugaki::ast::statement
