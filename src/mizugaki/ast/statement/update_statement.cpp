#include <mizugaki/ast/statement/update_statement.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::statement {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::rvalue_ptr;
using ::takatori::util::unique_object_ptr;

using common::clone_vector;

update_statement::update_statement(
        unique_object_ptr<name::name> table_name,
        common::vector<set_element> elements,
        unique_object_ptr<scalar::expression> where,
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
        region_type region) noexcept :
    update_statement {
            clone_unique(std::move(table_name)),
            elements,
            clone_unique(where),
            region,
    }
{}

update_statement::update_statement(update_statement const& other, object_creator creator) :
    update_statement {
            clone_unique(other.table_name_, creator),
            clone_vector(other.elements_, creator),
            clone_unique(other.where_, creator),
            other.region(),
    }
{}

update_statement::update_statement(update_statement&& other, object_creator creator) :
    update_statement {
            clone_unique(std::move(other.table_name_), creator),
            clone_vector(std::move(other.elements_), creator),
            clone_unique(std::move(other.where_), creator),
            other.region(),
    }
{}

update_statement* update_statement::clone(object_creator creator) const& {
    return creator.create_object<update_statement>(*this, creator);
}

update_statement* update_statement::clone(object_creator creator) && {
    return creator.create_object<update_statement>(std::move(*this), creator);
}

statement::node_kind_type update_statement::node_kind() const noexcept {
    return tag;
}

unique_object_ptr<name::name>& update_statement::table_name() noexcept {
    return table_name_;
}

unique_object_ptr<name::name> const& update_statement::table_name() const noexcept {
    return table_name_;
}

unique_object_ptr<scalar::expression>& update_statement::where() noexcept {
    return where_;
}

unique_object_ptr<scalar::expression> const& update_statement::where() const noexcept {
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
