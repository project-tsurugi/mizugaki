#include <mizugaki/ast/scalar/pattern_match_predicate.h>

#include <takatori/util/clonable.h>

namespace mizugaki::ast::scalar {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

pattern_match_predicate::pattern_match_predicate(
        operator_kind_type operator_kind,
        operand_type match_value,
        operand_type pattern,
        operand_type escape,
        not_type is_not,
        region_type region) noexcept:
    super { region },
    operator_kind_ { operator_kind },
    match_value_ { std::move(match_value) },
    pattern_ { std::move(pattern) },
    escape_ { std::move(escape) },
    is_not_ { is_not }
{}

pattern_match_predicate::pattern_match_predicate(pattern_match_predicate const& other, object_creator creator) :
    pattern_match_predicate {
            other.operator_kind_,
            clone_unique(other.match_value_, creator),
            clone_unique(other.pattern_, creator),
            clone_unique(other.escape_, creator),
            other.is_not_,
            other.region(),
    }
{}

pattern_match_predicate::pattern_match_predicate(pattern_match_predicate&& other, object_creator creator) :
    pattern_match_predicate {
            other.operator_kind_,
            clone_unique(std::move(other.match_value_), creator),
            clone_unique(std::move(other.pattern_), creator),
            clone_unique(std::move(other.escape_), creator),
            other.is_not_,
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

pattern_match_predicate::not_type& pattern_match_predicate::is_not() noexcept {
    return is_not_;
}

pattern_match_predicate::not_type const& pattern_match_predicate::is_not() const noexcept {
    return is_not_;
}

} // namespace mizugaki::ast::scalar
