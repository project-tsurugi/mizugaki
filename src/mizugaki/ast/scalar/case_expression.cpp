#include <mizugaki/ast/scalar/case_expression.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::scalar {

using when_clause = case_expression::when_clause;

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

using common::clone_vector;

case_expression::case_expression(
        operand_type operand,
        common::vector<when_clause> when_clauses,
        operand_type default_result,
        region_type region) noexcept :
    super { region },
    operand_ { std::move(operand) },
    when_clauses_ { std::move(when_clauses) },
    default_result_ { std::move(default_result) }
{}

case_expression::case_expression(case_expression const& other, object_creator creator) :
    case_expression {
            clone_unique(other.operand_, creator),
            clone_vector(other.when_clauses_, creator),
            clone_unique(other.default_result_, creator),
            other.region(),
    }
{}

case_expression::case_expression(case_expression&& other, object_creator creator) :
    case_expression {
            clone_unique(std::move(other.operand_), creator),
            clone_vector(std::move(other.when_clauses_), creator),
            clone_unique(std::move(other.default_result_), creator),
            other.region(),
    }
{}

case_expression* case_expression::clone(object_creator creator) const& {
    return creator.create_object<case_expression>(*this, creator);
}

case_expression* case_expression::clone(object_creator creator) && {
    return creator.create_object<case_expression>(std::move(*this), creator);
}

expression::node_kind_type case_expression::node_kind() const noexcept {
    return tag;
}

expression::operand_type& case_expression::operand() noexcept {
    return operand_;
}

expression::operand_type const& case_expression::operand() const noexcept {
    return operand_;
}

common::vector<when_clause>& case_expression::when_clauses() noexcept {
    return when_clauses_;
}

common::vector<when_clause> const& case_expression::when_clauses() const noexcept {
    return when_clauses_;
}

expression::operand_type& case_expression::default_result() noexcept {
    return default_result_;
}

expression::operand_type const& case_expression::default_result() const noexcept {
    return default_result_;
}

bool operator==(case_expression const& a, case_expression const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return false;
    }
    return eq(a.operand_, b.operand_)
            && eq(a.when_clauses_, b.when_clauses_)
            && eq(a.default_result_, b.default_result_);
}

bool operator!=(case_expression const& a, case_expression const& b) noexcept {
    return !(a == b);
}

bool case_expression::equals(expression const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<case_expression>(other);
}

} // namespace mizugaki::ast::scalar
