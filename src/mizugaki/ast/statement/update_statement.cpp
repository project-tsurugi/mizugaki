#include <mizugaki/ast/statement/update_statement.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::statement {

using ::takatori::util::clone_unique;
using ::takatori::util::rvalue_ptr;

using common::clone_vector;

update_statement::update_statement(
        std::unique_ptr<name::name> table_name,
        std::vector<set_element> elements,
        std::unique_ptr<scalar::expression> where,
        region_type region) noexcept :
    super { region },
    table_name_ { std::move(table_name) },
    elements_ { std::move(elements) },
    where_ { std::move(where) }
{}

update_statement::update_statement(
        name::name&& table_name,
        std::initializer_list<set_element> elements,
        rvalue_ptr<scalar::expression> where,
        region_type region) :
    update_statement {
            clone_unique(std::move(table_name)),
            elements,
            clone_unique(where),
            region,
    }
{}

update_statement::update_statement(::takatori::util::clone_tag_t, update_statement const& other) :
    update_statement {
            clone_unique(other.table_name_),
            clone_vector(other.elements_),
            clone_unique(other.where_),
            other.region(),
    }
{}

update_statement::update_statement(::takatori::util::clone_tag_t, update_statement&& other) :
    update_statement {
            clone_unique(std::move(other.table_name_)),
            clone_vector(std::move(other.elements_)),
            clone_unique(std::move(other.where_)),
            other.region(),
    }
{}

update_statement* update_statement::clone() const& {
    return new update_statement(::takatori::util::clone_tag, *this); // NOLINT
}

update_statement* update_statement::clone() && {
    return new update_statement(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

statement::node_kind_type update_statement::node_kind() const noexcept {
    return tag;
}

std::unique_ptr<name::name>& update_statement::table_name() noexcept {
    return table_name_;
}

std::unique_ptr<name::name> const& update_statement::table_name() const noexcept {
    return table_name_;
}

std::unique_ptr<scalar::expression>& update_statement::where() noexcept {
    return where_;
}

std::unique_ptr<scalar::expression> const& update_statement::where() const noexcept {
    return where_;
}

bool operator==(update_statement const& a, update_statement const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return true;
    }
    return eq(a.table_name_, b.table_name_)
            && eq(a.elements_, b.elements_)
            && eq(a.where_, b.where_);
}

bool operator!=(update_statement const& a, update_statement const& b) noexcept {
    return !(a == b);
}

bool update_statement::equals(statement const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void update_statement::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "table_name"sv, table_name_);
    property(acceptor, "elements"sv, elements_);
    property(acceptor, "where"sv, where_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, update_statement const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::statement
