#include <mizugaki/ast/statement/empty_statement.h>

namespace mizugaki::ast::statement {

using ::takatori::util::object_creator;

empty_statement::empty_statement(
        region_type region) noexcept :
    super { region }
{}

empty_statement::empty_statement(empty_statement const& other, object_creator) :
    empty_statement {
            other.region(),
    }
{}

empty_statement::empty_statement(empty_statement&& other, object_creator) :
    empty_statement {
            other.region(),
    }
{}

empty_statement* empty_statement::clone(object_creator creator) const& {
    return creator.create_object<empty_statement>(*this, creator);
}

empty_statement* empty_statement::clone(object_creator creator) && {
    return creator.create_object<empty_statement>(std::move(*this), creator);
}

statement::node_kind_type empty_statement::node_kind() const noexcept {
    return tag;
}

} // namespace mizugaki::ast::statement
