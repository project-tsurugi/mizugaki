#include <mizugaki/ast/statement/insert_statement.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>
#include <mizugaki/ast/common/vector.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::statement {

using ::takatori::util::clone_unique;
using ::takatori::util::rvalue_ptr;

using common::clone_vector;
using common::to_vector;

insert_statement::insert_statement(
        std::unique_ptr<name::name> table_name,
        std::vector<std::unique_ptr<name::simple>> columns,
        std::unique_ptr<query::expression> expression,
        std::vector<option_type> options,
        region_type region) noexcept :
    super { region },
    table_name_ { std::move(table_name) },
    columns_ { std::move(columns) },
    expression_ { std::move(expression) },
    options_ {std::move(options) }
{}

insert_statement::insert_statement(
        name::name&& table_name,
        common::rvalue_list<name::simple> columns,
        rvalue_ptr<query::expression> expression,
        std::initializer_list<option_type> options,
        region_type region) :
    insert_statement {
            clone_unique(std::move(table_name)),
            to_vector(columns),
            clone_unique(expression),
            options,
            region,
    }
{}

insert_statement::insert_statement(::takatori::util::clone_tag_t, insert_statement const& other) :
    insert_statement {
            clone_unique(other.table_name_),
            clone_vector(other.columns_),
            clone_unique(other.expression_),
            other.options_,
            other.region(),
    }
{}

insert_statement::insert_statement(::takatori::util::clone_tag_t, insert_statement&& other) :
    insert_statement {
            clone_unique(std::move(other.table_name_)),
            clone_vector(std::move(other.columns_)),
            clone_unique(std::move(other.expression_)),
            std::move(other.options_),
            other.region(),
    }
{}

insert_statement* insert_statement::clone() const& {
    return new insert_statement(::takatori::util::clone_tag, *this); // NOLINT
}

insert_statement* insert_statement::clone() && {
    return new insert_statement(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

statement::node_kind_type insert_statement::node_kind() const noexcept {
    return tag;
}

std::unique_ptr<name::name>& insert_statement::table_name() noexcept {
    return table_name_;
}

std::unique_ptr<name::name> const& insert_statement::table_name() const noexcept {
    return table_name_;
}

std::vector<std::unique_ptr<name::simple>>& insert_statement::columns() noexcept {
    return columns_;
}

std::vector<std::unique_ptr<name::simple>> const& insert_statement::columns() const noexcept {
    return columns_;
}

std::unique_ptr<query::expression>& insert_statement::expression() noexcept {
    return expression_;
}

std::unique_ptr<query::expression> const& insert_statement::expression() const noexcept {
    return expression_;
}

std::vector<insert_statement::option_type>& insert_statement::options() noexcept {
    return options_;
}

std::vector<insert_statement::option_type> const& insert_statement::options() const noexcept {
    return options_;
}

bool operator==(insert_statement const& a, insert_statement const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return true;
    }
    return eq(a.table_name_, b.table_name_)
        && eq(a.columns_, b.columns_)
        && eq(a.expression_, b.expression_)
        && eq(a.options_, b.options_);
}

bool operator!=(insert_statement const& a, insert_statement const& b) noexcept {
    return !(a == b);
}

bool insert_statement::equals(statement const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void insert_statement::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "table_name"sv, table_name_);
    property(acceptor, "columns"sv, columns_);
    property(acceptor, "expression"sv, expression_);
    property(acceptor, "options"sv, options_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, insert_statement const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::statement
