#include <mizugaki/ast/table/unnest.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::table {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

unnest::unnest(
        unique_object_ptr<scalar::expression> expression,
        bool_type with_ordinality,
        region_type region) noexcept :
    super { region },
    expression_ { std::move(expression) },
    with_ordinality_ { with_ordinality }
{}

unnest::unnest(unnest const& other, object_creator creator) :
    unnest {
            clone_unique(other.expression_, creator),
            other.with_ordinality_,
            other.region(),
    }
{}

unnest::unnest(unnest&& other, object_creator creator) :
    unnest {
            clone_unique(other.expression_, creator),
            other.with_ordinality_,
            other.region(),
    }
{}

unnest* unnest::clone(object_creator creator) const& {
    return creator.create_object<unnest>(*this, creator);
}

unnest* unnest::clone(object_creator creator) && {
    return creator.create_object<unnest>(std::move(*this), creator);
}

expression::node_kind_type unnest::node_kind() const noexcept {
    return tag;
}

unique_object_ptr<scalar::expression>& unnest::expression() noexcept {
    return expression_;
}

unique_object_ptr<scalar::expression> const& unnest::expression() const noexcept {
    return expression_;
}

unnest::bool_type& unnest::with_ordinality() noexcept {
    return with_ordinality_;
}

unnest::bool_type const& unnest::with_ordinality() const noexcept {
    return with_ordinality_;
}

bool operator==(unnest const& a, unnest const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return true;
    }
    return eq(a.expression_, b.expression_)
            && eq(a.with_ordinality_, b.with_ordinality_);
}

bool operator!=(unnest const& a, unnest const& b) noexcept {
    return !(a == b);
}

bool unnest::equals(table::expression const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

} // namespace mizugaki::ast::table
