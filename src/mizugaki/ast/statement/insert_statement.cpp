#include <mizugaki/ast/statement/insert_statement.h>

#include <takatori/util/clonable.h>

namespace mizugaki::ast::statement {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

using common::clone_vector;

insert_statement::insert_statement(
        unique_object_ptr<name::name> table_name,
        common::vector<unique_object_ptr<name::simple>> columns,
        unique_object_ptr<query::expression> expression,
        region_type region) noexcept :
    super { region },
    table_name_ { std::move(table_name) },
    columns_ { std::move(columns) },
    expression_ { std::move(expression) }
{}

insert_statement::insert_statement(insert_statement const& other, object_creator creator) :
    insert_statement {
            clone_unique(other.table_name_, creator),
            clone_vector(other.columns_, creator),
            clone_unique(other.expression_, creator),
            other.region(),
    }
{}

insert_statement::insert_statement(insert_statement&& other, object_creator creator) :
    insert_statement {
            clone_unique(std::move(other.table_name_), creator),
            clone_vector(std::move(other.columns_), creator),
            clone_unique(std::move(other.expression_), creator),
            other.region(),
    }
{}

insert_statement* insert_statement::clone(object_creator creator) const& {
    return creator.create_object<insert_statement>(*this, creator);
}

insert_statement* insert_statement::clone(object_creator creator) && {
    return creator.create_object<insert_statement>(std::move(*this), creator);
}

statement::node_kind_type insert_statement::node_kind() const noexcept {
    return tag;
}

unique_object_ptr<name::name>& insert_statement::table_name() noexcept {
    return table_name_;
}

unique_object_ptr<name::name> const& insert_statement::table_name() const noexcept {
    return table_name_;
}

common::vector<unique_object_ptr<name::simple>>& insert_statement::columns() noexcept {
    return columns_;
}

common::vector<unique_object_ptr<name::simple>> const& insert_statement::columns() const noexcept {
    return columns_;
}

unique_object_ptr<query::expression>& insert_statement::expression() noexcept {
    return expression_;
}

unique_object_ptr<query::expression> const& insert_statement::expression() const noexcept {
    return expression_;
}

} // namespace mizugaki::ast::statement
