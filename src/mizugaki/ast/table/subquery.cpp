#include <mizugaki/ast/table/subquery.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::table {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

subquery::subquery(
        unique_object_ptr<query::expression> body,
        correlation_type correlation,
        bool_type is_lateral,
        region_type region) noexcept :
    super { region },
    body_ { std::move(body) },
    correlation_ { std::move(correlation) },
    is_lateral_ { is_lateral }
{}

subquery::subquery(subquery const& other, object_creator creator) :
    subquery {
            clone_unique(other.body_, creator),
            decltype(correlation_) { other.correlation_, creator },
            other.is_lateral_,
            other.region(),
    }
{}

subquery::subquery(subquery&& other, object_creator creator) :
    subquery {
            clone_unique(std::move(other.body_), creator),
            decltype(correlation_) { std::move(other.correlation_), creator },
            other.is_lateral_,
            other.region(),
    }
{}

subquery* subquery::clone(object_creator creator) const& {
    return creator.create_object<subquery>(*this, creator);
}

subquery* subquery::clone(object_creator creator) && {
    return creator.create_object<subquery>(std::move(*this), creator);
}

expression::node_kind_type subquery::node_kind() const noexcept {
    return tag;
}

unique_object_ptr<query::expression>& subquery::body() noexcept {
    return body_;
}

unique_object_ptr<query::expression> const& subquery::body() const noexcept {
    return body_;
}

unique_object_ptr<query::expression>& subquery::operator*() noexcept {
    return body();
}

unique_object_ptr<query::expression> const& subquery::operator*() const noexcept {
    return body();
}

subquery::correlation_type& subquery::correlation() noexcept {
    return correlation_;
}

subquery::correlation_type const& subquery::correlation() const noexcept {
    return correlation_;
}

subquery::bool_type& subquery::is_lateral() noexcept {
    return is_lateral_;
}

subquery::bool_type const& subquery::is_lateral() const noexcept {
    return is_lateral_;
}

bool operator==(subquery const& a, subquery const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return true;
    }
    return eq(a.body_, b.body_)
            && eq(a.correlation_, b.correlation_)
            && eq(a.is_lateral_, b.is_lateral_);
}

bool operator!=(subquery const& a, subquery const& b) noexcept {
    return !(a == b);
}

bool subquery::equals(expression const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<subquery>(other);
}

} // namespace mizugaki::ast::table
