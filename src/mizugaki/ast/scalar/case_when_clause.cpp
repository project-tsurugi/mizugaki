#include <mizugaki/ast/scalar/case_when_clause.h>

#include <takatori/util/clonable.h>

namespace mizugaki::ast::scalar {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

case_when_clause::case_when_clause(
        operand_type when,
        operand_type result,
        region_type region) noexcept:
    element { region },
    when_ { std::move(when) },
    result_ { std::move(result) }
{}

case_when_clause::case_when_clause(case_when_clause const& other, object_creator creator) :
    case_when_clause {
            clone_unique(*other.when_, creator),
            clone_unique(*other.result_, creator),
            other.region(),
    }
{}

case_when_clause::case_when_clause(case_when_clause&& other, object_creator creator) :
    case_when_clause {
            clone_unique(std::move(*other.when_), creator),
            clone_unique(std::move(*other.result_), creator),
            other.region(),
    }
{}

expression::operand_type& case_when_clause::when() noexcept {
    return *when_;
}

expression::operand_type const& case_when_clause::when() const noexcept {
    return *when_;
}

expression::operand_type& case_when_clause::result() noexcept {
    return *result_;
}

expression::operand_type const& case_when_clause::result() const noexcept {
    return *result_;
}

} // namespace mizugaki::ast::scalar
