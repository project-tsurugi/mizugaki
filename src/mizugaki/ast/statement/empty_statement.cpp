#include <mizugaki/ast/statement/empty_statement.h>

#include <mizugaki/ast/compare_utils.h>

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

bool operator==(empty_statement const&, empty_statement const&) noexcept {
    return true;
}

bool operator!=(empty_statement const& a, empty_statement const& b) noexcept {
    return !(a == b);
}

bool empty_statement::equals(statement const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

} // namespace mizugaki::ast::statement
