#include <mizugaki/ast/statement/empty_statement.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::statement {


empty_statement::empty_statement(
        region_type region) noexcept :
    super { region }
{}

empty_statement::empty_statement(::takatori::util::clone_tag_t, empty_statement const& other) :
    empty_statement {
            other.region(),
    }
{}

empty_statement::empty_statement(::takatori::util::clone_tag_t, empty_statement&& other) :
    empty_statement {
            other.region(),
    }
{}

empty_statement* empty_statement::clone() const& {
    return new empty_statement(::takatori::util::clone_tag, *this); // NOLINT
}

empty_statement* empty_statement::clone() && {
    return new empty_statement(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
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

void empty_statement::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, empty_statement const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::statement
