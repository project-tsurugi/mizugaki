#include <mizugaki/ast/scalar/case_when_clause.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::scalar {

using ::takatori::util::clone_unique;

case_when_clause::case_when_clause(
        operand_type when,
        operand_type result,
        region_type region) noexcept :
    element { region },
    when_ { std::move(when) },
    result_ { std::move(result) }
{}

case_when_clause::case_when_clause(
        expression&& when,
        expression&& result,
        region_type region) :
    case_when_clause {
            clone_unique(std::move(when)),
            clone_unique(std::move(result)),
            region,
    }
{}

case_when_clause::case_when_clause(::takatori::util::clone_tag_t, case_when_clause const& other) :
    case_when_clause {
            clone_unique(*other.when_),
            clone_unique(*other.result_),
            other.region(),
    }
{}

case_when_clause::case_when_clause(::takatori::util::clone_tag_t, case_when_clause&& other) :
    case_when_clause {
            clone_unique(std::move(*other.when_)),
            clone_unique(std::move(*other.result_)),
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

bool operator==(case_when_clause const& a, case_when_clause const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return false;
    }
    return eq(*a.when_, *b.when_)
            && eq(*a.result_, *b.result_);
}

bool operator!=(case_when_clause const& a, case_when_clause const& b) noexcept {
    return !(a == b);
}

::takatori::serializer::object_acceptor& operator<<(::takatori::serializer::object_acceptor& acceptor, case_when_clause const& value) {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor);
    property(acceptor, "when"sv, *value.when_);
    property(acceptor, "result"sv, *value.result_);
    region_property(acceptor, value);
    return acceptor;
}

std::ostream& operator<<(std::ostream& out, case_when_clause const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::scalar
