#include <mizugaki/ast/scalar/subquery.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::scalar {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

subquery::subquery(
        unique_object_ptr<query::expression> body,
        region_type region) noexcept :
    super { region },
    body_ { std::move(body) }
{}

subquery::subquery(subquery const& other, object_creator creator) :
    subquery {
            clone_unique(other.body_, creator),
            other.region(),
    }
{}

subquery::subquery(subquery&& other, object_creator creator) :
    subquery {
            clone_unique(std::move(other.body_), creator),
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

bool operator==(subquery const& a, subquery const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return false;
    }
    return eq(a.body_, b.body_);
}

bool operator!=(subquery const& a, subquery const& b) noexcept {
    return !(a == b);
}

bool subquery::equals(expression const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<subquery>(other);
}

} // namespace mizugaki::ast::scalar
