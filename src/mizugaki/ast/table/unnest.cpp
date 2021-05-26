#include <mizugaki/ast/table/unnest.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::table {

using ::takatori::util::clone_unique;

unnest::unnest(
        std::unique_ptr<scalar::expression> expression,
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

unnest::unnest(::takatori::util::clone_tag_t, unnest const& other) :
    unnest {
            clone_unique(other.expression_),
            other.with_ordinality_,
            decltype(correlation_) { other.correlation_ },
            other.region(),
    }
{}

unnest::unnest(::takatori::util::clone_tag_t, unnest&& other) :
    unnest {
            clone_unique(other.expression_),
            other.with_ordinality_,
            decltype(correlation_) { std::move(other.correlation_) },
            other.region(),
    }
{}

unnest* unnest::clone() const& {
    return new unnest(::takatori::util::clone_tag, *this); // NOLINT
}

unnest* unnest::clone() && {
    return new unnest(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

expression::node_kind_type unnest::node_kind() const noexcept {
    return tag;
}

std::unique_ptr<scalar::expression>& unnest::expression() noexcept {
    return expression_;
}

std::unique_ptr<scalar::expression> const& unnest::expression() const noexcept {
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
