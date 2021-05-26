#include <mizugaki/ast/scalar/case_expression.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::scalar {

using when_clause = case_expression::when_clause;

using ::takatori::util::clone_unique;
using ::takatori::util::rvalue_ptr;

using common::clone_vector;

case_expression::case_expression(
        operand_type operand,
        std::vector<when_clause> when_clauses,
        operand_type default_result,
        region_type region) noexcept :
    super { region },
    operand_ { std::move(operand) },
    when_clauses_ { std::move(when_clauses) },
    default_result_ { std::move(default_result) }
{}

case_expression::case_expression(
        expression&& operand,
        std::initializer_list<when_clause> when_clauses,
        rvalue_ptr<expression> default_result,
        region_type region) :
    case_expression {
            clone_unique(std::move(operand)),
            when_clauses,
            clone_unique(default_result),
            region,
    }
{}

case_expression::case_expression(
        std::initializer_list<when_clause> when_clauses,
        rvalue_ptr<expression> default_result,
        region_type region) :
    case_expression {
            nullptr,
            when_clauses,
            clone_unique(default_result),
            region,
    }
{}

case_expression::case_expression(::takatori::util::clone_tag_t, case_expression const& other) :
    case_expression {
            clone_unique(other.operand_),
            clone_vector(other.when_clauses_),
            clone_unique(other.default_result_),
            other.region(),
    }
{}

case_expression::case_expression(::takatori::util::clone_tag_t, case_expression&& other) :
    case_expression {
            clone_unique(std::move(other.operand_)),
            clone_vector(std::move(other.when_clauses_)),
            clone_unique(std::move(other.default_result_)),
            other.region(),
    }
{}

case_expression* case_expression::clone() const& {
    return new case_expression(::takatori::util::clone_tag, *this); // NOLINT
}

case_expression* case_expression::clone() && {
    return new case_expression(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
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

std::vector<when_clause>& case_expression::when_clauses() noexcept {
    return when_clauses_;
}

std::vector<when_clause> const& case_expression::when_clauses() const noexcept {
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
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void case_expression::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "operand"sv, operand_);
    property(acceptor, "when_clauses"sv, when_clauses_);
    property(acceptor, "default_result"sv, default_result_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, case_expression const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::scalar
