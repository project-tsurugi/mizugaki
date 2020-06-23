#include <mizugaki/ast/table/unnest.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::table {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

unnest::unnest(
        unique_object_ptr<scalar::expression> expression,
        bool_type with_ordinality,
        correlation_type correlation,
        region_type region) noexcept :
    super { region },
    expression_ { std::move(expression) },
    with_ordinality_ { with_ordinality },
    correlation_ { std::move(correlation) }
{}

unnest::unnest(
        scalar::expression&& expression,
        correlation_type correlation,
        bool_type with_ordinality,
        region_type region) :
    unnest {
            clone_unique(std::move(expression)),
            with_ordinality,
            std::move(correlation),
            region,
    }
{}

unnest::unnest(unnest const& other, object_creator creator) :
    unnest {
            clone_unique(other.expression_, creator),
            other.with_ordinality_,
            decltype(correlation_) { other.correlation_, creator },
            other.region(),
    }
{}

unnest::unnest(unnest&& other, object_creator creator) :
    unnest {
            clone_unique(other.expression_, creator),
            other.with_ordinality_,
            decltype(correlation_) { std::move(other.correlation_), creator },
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

void unnest::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "expression"sv, expression_);
    property(acceptor, "with_ordinality"sv, with_ordinality_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, unnest const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::table
