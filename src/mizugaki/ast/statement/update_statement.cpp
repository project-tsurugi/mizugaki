#include <mizugaki/ast/statement/update_statement.h>

#include <takatori/util/clonable.h>

namespace mizugaki::ast::statement {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

using common::clone_vector;

update_statement::update_statement(
        unique_object_ptr<name::name> table_name,
        common::vector<set_element> elements,
        unique_object_ptr<scalar::expression> condition,
        region_type region) noexcept :
    super { region },
    table_name_ { std::move(table_name) },
    elements_ { std::move(elements) },
    condition_ { std::move(condition) }
{}

update_statement::update_statement(update_statement const& other, object_creator creator) :
    update_statement {
            clone_unique(other.table_name_, creator),
            clone_vector(other.elements_, creator),
            clone_unique(other.condition_, creator),
            other.region(),
    }
{}

update_statement::update_statement(update_statement&& other, object_creator creator) :
    update_statement {
            clone_unique(std::move(other.table_name_), creator),
            clone_vector(std::move(other.elements_), creator),
            clone_unique(std::move(other.condition_), creator),
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

unique_object_ptr<scalar::expression>& update_statement::condition() noexcept {
    return condition_;
}

unique_object_ptr<scalar::expression> const& update_statement::condition() const noexcept {
    return condition_;
}

} // namespace mizugaki::ast::statement