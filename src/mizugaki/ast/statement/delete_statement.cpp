#include <mizugaki/ast/statement/delete_statement.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::statement {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::rvalue_ptr;
using ::takatori::util::unique_object_ptr;

delete_statement::delete_statement(
        unique_object_ptr<name::name> table_name,
        unique_object_ptr<scalar::expression> where,
        region_type region) noexcept :
    super { region },
    table_name_ { std::move(table_name) },
    where_ { std::move(where) }
{}

delete_statement::delete_statement(
        name::name&& table_name,
        rvalue_ptr<scalar::expression> where,
        region_type region) :
    delete_statement {
            clone_unique(std::move(table_name)),
            clone_unique(where),
            region,
    }
{}

delete_statement::delete_statement(delete_statement const& other, object_creator creator) :
    delete_statement {
            clone_unique(other.table_name_, creator),
            clone_unique(other.where_, creator),
            other.region(),
    }
{}

delete_statement::delete_statement(delete_statement&& other, object_creator creator) :
    delete_statement {
            clone_unique(std::move(other.table_name_), creator),
            clone_unique(std::move(other.where_), creator),
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

unique_object_ptr<scalar::expression>& delete_statement::where() noexcept {
    return where_;
}

unique_object_ptr<scalar::expression> const& delete_statement::where() const noexcept {
    return where_;
}

bool operator==(delete_statement const& a, delete_statement const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return true;
    }
    return eq(a.table_name_, b.table_name_)
        && eq(a.where_, b.where_);
}

bool operator!=(delete_statement const& a, delete_statement const& b) noexcept {
    return !(a == b);
}

bool delete_statement::equals(statement const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void delete_statement::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "table_name"sv, table_name_);
    property(acceptor, "where"sv, where_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, delete_statement const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::statement
