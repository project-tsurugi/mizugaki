#include <mizugaki/ast/scalar/pattern_match_predicate.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::scalar {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

pattern_match_predicate::pattern_match_predicate(
        operand_type match_value,
        bool_type is_not,
        operator_kind_type operator_kind,
        operand_type pattern,
        operand_type escape,
        region_type region) noexcept:
    super { region },
    match_value_ { std::move(match_value) },
    is_not_ { is_not },
    operator_kind_ { operator_kind },
    pattern_ { std::move(pattern) },
    escape_ { std::move(escape) }
{}

pattern_match_predicate::pattern_match_predicate(pattern_match_predicate const& other, object_creator creator) :
    pattern_match_predicate {
            clone_unique(other.match_value_, creator),
            other.is_not_,
            other.operator_kind_,
            clone_unique(other.pattern_, creator),
            clone_unique(other.escape_, creator),
            other.region(),
    }
{}

pattern_match_predicate::pattern_match_predicate(pattern_match_predicate&& other, object_creator creator) :
    pattern_match_predicate {
            clone_unique(std::move(other.match_value_), creator),
            other.is_not_,
            other.operator_kind_,
            clone_unique(std::move(other.pattern_), creator),
            clone_unique(std::move(other.escape_), creator),
            other.region(),
    }
{}

pattern_match_predicate* pattern_match_predicate::clone(object_creator creator) const& {
    return creator.create_object<pattern_match_predicate>(*this, creator);
}

pattern_match_predicate* pattern_match_predicate::clone(object_creator creator) && {
    return creator.create_object<pattern_match_predicate>(std::move(*this), creator);
}

expression::node_kind_type pattern_match_predicate::node_kind() const noexcept {
    return tag;
}

pattern_match_predicate::operator_kind_type& pattern_match_predicate::operator_kind() noexcept {
    return operator_kind_;
}

pattern_match_predicate::operator_kind_type const& pattern_match_predicate::operator_kind() const noexcept {
    return operator_kind_;
}

expression::operand_type& pattern_match_predicate::match_value() noexcept {
    return match_value_;
}

expression::operand_type const& pattern_match_predicate::match_value() const noexcept {
    return match_value_;
}

expression::operand_type& pattern_match_predicate::pattern() noexcept {
    return pattern_;
}

expression::operand_type const& pattern_match_predicate::pattern() const noexcept {
    return pattern_;
}

expression::operand_type& pattern_match_predicate::escape() noexcept {
    return escape_;
}

expression::operand_type const& pattern_match_predicate::escape() const noexcept {
    return escape_;
}

pattern_match_predicate::bool_type& pattern_match_predicate::is_not() noexcept {
    return is_not_;
}

pattern_match_predicate::bool_type const& pattern_match_predicate::is_not() const noexcept {
    return is_not_;
}

bool operator==(pattern_match_predicate const& a, pattern_match_predicate const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return false;
    }
    return eq(a.operator_kind_, b.operator_kind_)
            && eq(a.match_value_, b.match_value_)
            && eq(a.pattern_, b.pattern_)
            && eq(a.escape_, b.escape_)
            && eq(a.is_not_, b.is_not_);
}

bool operator!=(pattern_match_predicate const& a, pattern_match_predicate const& b) noexcept {
    return !(a == b);
}

bool pattern_match_predicate::equals(expression const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<pattern_match_predicate>(other);
}

} // namespace mizugaki::ast::scalar
