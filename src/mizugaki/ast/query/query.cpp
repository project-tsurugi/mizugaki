#include <mizugaki/ast/query/query.h>

#include <mizugaki/ast/common/optional.h>
#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::query {

using ::takatori::util::clone_unique;
using ::takatori::util::rvalue_ptr;

using common::clone_optional;
using common::clone_vector;
using common::to_vector;

query::query(
        std::optional<quantifier_type> quantifier,
        std::vector<std::unique_ptr<select_element>> elements,
        std::vector<std::unique_ptr<table::expression>> from,
        std::unique_ptr<scalar::expression> where,
        std::optional<group_by_clause> group_by,
        std::unique_ptr<scalar::expression> having,
        std::vector<common::sort_element> order_by,
        std::unique_ptr<scalar::expression> limit,
        region_type region) noexcept :
    super { region },
    quantifier_ { quantifier },
    elements_ { std::move(elements) },
    from_ { std::move(from) },
    where_ { std::move(where) },
    group_by_ { std::move(group_by) },
    having_ { std::move(having) },
    order_by_ { std::move(order_by) },
    limit_ { std::move(limit) }
{}

query::query(
        common::rvalue_list<select_element> elements,
        common::rvalue_list<table::expression> from,
        rvalue_ptr<scalar::expression> where,
        std::optional<group_by_clause> group_by,
        rvalue_ptr<scalar::expression> having,
        std::initializer_list<common::sort_element> order_by,
        rvalue_ptr<scalar::expression> limit,
        element::region_type region) :
    query {
            std::nullopt,
            to_vector(elements),
            to_vector(from),
            clone_unique(where),
            std::move(group_by),
            clone_unique(having),
            decltype(order_by_) { order_by },
            clone_unique(limit),
            region,
    }
{}

query::query(
        std::optional<quantifier_type> quantifier,
        common::rvalue_list<select_element> elements,
        common::rvalue_list<table::expression> from,
        rvalue_ptr<scalar::expression> where,
        std::optional<group_by_clause> group_by,
        rvalue_ptr<scalar::expression> having,
        std::initializer_list<common::sort_element> order_by,
        rvalue_ptr<scalar::expression> limit,
        element::region_type region) :
    query {
            quantifier,
            to_vector(elements),
            to_vector(from),
            clone_unique(where),
            std::move(group_by),
            clone_unique(having),
            decltype(order_by_) { order_by },
            clone_unique(limit),
            region,
    }
{}

query::query(::takatori::util::clone_tag_t, query const& other) :
    query {
            other.quantifier_,
            clone_vector(other.elements_),
            clone_vector(other.from_),
            clone_unique(other.where_),
            clone_optional(other.group_by_),
            clone_unique(other.having_),
            clone_vector(other.order_by_),
            clone_unique(other.limit_),
            other.region(),
    }
{}

query::query(::takatori::util::clone_tag_t, query&& other) :
    query {
            other.quantifier_,
            clone_vector(std::move(other.elements_)),
            clone_vector(std::move(other.from_)),
            clone_unique(std::move(other.where_)),
            clone_optional(std::move(other.group_by_)),
            clone_unique(std::move(other.having_)),
            clone_vector(std::move(other.order_by_)),
            clone_unique(std::move(other.limit_)),
            other.region(),
    }
{}

query* query::clone() const& {
    return new query(::takatori::util::clone_tag, *this); // NOLINT
}

query* query::clone()&& {
    return new query(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

expression::node_kind_type query::node_kind() const noexcept {
    return tag;
}

std::vector<std::unique_ptr<select_element>>& query::elements() noexcept {
    return elements_;
}

std::vector<std::unique_ptr<select_element>> const& query::elements() const noexcept {
    return elements_;
}

std::vector<std::unique_ptr<table::expression>>& query::from() noexcept {
    return from_;
}

std::vector<std::unique_ptr<table::expression>> const& query::from() const noexcept {
    return from_;
}

std::unique_ptr<scalar::expression>& query::where() noexcept {
    return where_;
}

std::unique_ptr<scalar::expression> const& query::where() const noexcept {
    return where_;
}

std::optional<group_by_clause>& query::group_by() noexcept {
    return group_by_;
}

std::optional<group_by_clause> const& query::group_by() const noexcept {
    return group_by_;
}

std::unique_ptr<scalar::expression>& query::having() noexcept {
    return having_;
}

std::unique_ptr<scalar::expression> const& query::having() const noexcept {
    return having_;
}

std::vector<common::sort_element>& query::order_by() noexcept {
    return order_by_;
}

std::vector<common::sort_element> const& query::order_by() const noexcept {
    return order_by_;
}

std::unique_ptr<scalar::expression>& query::limit() noexcept {
    return limit_;
}

std::unique_ptr<scalar::expression> const& query::limit() const noexcept {
    return limit_;
}

std::optional<query::quantifier_type>& query::quantifier() noexcept {
    return quantifier_;
}

std::optional<query::quantifier_type> const& query::quantifier() const noexcept {
    return quantifier_;
}

bool operator==(query const& a, query const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return false;
    }
    return eq(a.elements_, b.elements_)
            && eq(a.from_, b.from_)
            && eq(a.where_, b.where_)
            && eq(a.group_by_, b.group_by_)
            && eq(a.having_, b.having_)
            && eq(a.order_by_, b.order_by_)
            && eq(a.limit_, b.limit_)
            && eq(a.quantifier_, b.quantifier_);
}

bool operator!=(query const& a, query const& b) noexcept {
    return !(a == b);
}

bool query::equals(expression const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void query::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "quantifier"sv, quantifier_);
    property(acceptor, "elements"sv, elements_);
    property(acceptor, "from"sv, from_);
    property(acceptor, "where"sv, where_);
    property(acceptor, "group_by"sv, group_by_);
    property(acceptor, "having"sv, having_);
    property(acceptor, "order_by"sv, order_by_);
    property(acceptor, "limit"sv, limit_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, query const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::query
