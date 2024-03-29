#include <mizugaki/ast/statement/select_statement.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::statement {

using ::takatori::util::clone_unique;

using common::clone_vector;

select_statement::select_statement(
        std::unique_ptr<query::expression> expression,
        std::vector<target_element> targets,
        region_type region) noexcept :
    super { region },
    expression_ { std::move(expression) },
    targets_ { std::move(targets) }
{}

select_statement::select_statement(
        query::expression&& expression,
        std::initializer_list<target_element> targets,
        region_type region) :
    select_statement {
            clone_unique(std::move(expression)),
            decltype(targets_) { targets },
            region,
    }
{}

select_statement::select_statement(::takatori::util::clone_tag_t, select_statement const& other) :
    select_statement {
            clone_unique(other.expression_),
            clone_vector(other.targets_),
            other.region(),
    }
{}

select_statement::select_statement(::takatori::util::clone_tag_t, select_statement&& other) :
    select_statement {
            clone_unique(std::move(other.expression_)),
            clone_vector(std::move(other.targets_)),
            other.region(),
    }
{}

select_statement* select_statement::clone() const& {
    return new select_statement(::takatori::util::clone_tag, *this); // NOLINT
}

select_statement* select_statement::clone() && {
    return new select_statement(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

statement::node_kind_type select_statement::node_kind() const noexcept {
    return tag;
}

std::unique_ptr<query::expression>& select_statement::expression() noexcept {
    return expression_;
}

std::unique_ptr<query::expression> const& select_statement::expression() const noexcept {
    return expression_;
}

std::vector<target_element>& select_statement::targets() noexcept {
    return targets_;
}

std::vector<target_element> const& select_statement::targets() const noexcept {
    return targets_;
}

bool operator==(select_statement const& a, select_statement const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return true;
    }
    return eq(a.expression_, b.expression_)
            && eq(a.targets_, b.targets_);
}

bool operator!=(select_statement const& a, select_statement const& b) noexcept {
    return !(a == b);
}

bool select_statement::equals(statement const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void select_statement::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "expression"sv, expression_);
    property(acceptor, "targets"sv, targets_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, select_statement const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::statement
