#include <mizugaki/ast/query/query.h>

#include <mizugaki/ast/common/optional.h>

namespace mizugaki::ast::query {

using ::takatori::util::clone_unique;
using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

using common::clone_optional;
using common::clone_vector;

query::query(
        common::vector<unique_object_ptr<select_element>> elements,
        common::vector<unique_object_ptr<table::expression>> from,
        unique_object_ptr<scalar::expression> where,
        std::optional<group_by_clause> group_by,
        unique_object_ptr<scalar::expression> having,
        common::vector<common::sort_element> order_by,
        unique_object_ptr<scalar::expression> limit,
        std::optional<quantifier_type> quantifier,
        region_type region) noexcept :
    super { region },
    elements_ { std::move(elements) },
    from_ { std::move(from) },
    where_ { std::move(where) },
    group_by_ { std::move(group_by) },
    having_ { std::move(having) },
    order_by_ { std::move(order_by) },
    limit_ { std::move(limit) },
    quantifier_ { std::move(quantifier) }
{}

query::query(query const& other, object_creator creator) :
    query {
            clone_vector(other.elements_, creator),
            clone_vector(other.from_, creator),
            clone_unique(other.where_, creator),
            clone_optional(other.group_by_, creator),
            clone_unique(other.having_, creator),
            clone_vector(other.order_by_, creator),
            clone_unique(other.limit_, creator),
            other.quantifier_,
            other.region(),
    }
{}

query::query(query&& other, object_creator creator) :
    query {
            clone_vector(std::move(other.elements_), creator),
            clone_vector(std::move(other.from_), creator),
            clone_unique(std::move(other.where_), creator),
            clone_optional(std::move(other.group_by_), creator),
            clone_unique(std::move(other.having_), creator),
            clone_vector(std::move(other.order_by_), creator),
            clone_unique(std::move(other.limit_), creator),
            std::move(other.quantifier_),
            other.region(),
    }
{}

query* query::clone(object_creator creator) const& {
    return creator.create_object<query>(*this, creator);
}

query* query::clone(object_creator creator)&& {
    return creator.create_object<query>(std::move(*this), creator);
}

expression::node_kind_type query::node_kind() const noexcept {
    return tag;
}

common::vector<unique_object_ptr<select_element>>& query::elements() noexcept {
    return elements_;
}

common::vector<unique_object_ptr<select_element>> const& query::elements() const noexcept {
    return elements_;
}

common::vector<unique_object_ptr<table::expression>>& query::from() noexcept {
    return from_;
}

common::vector<unique_object_ptr<table::expression>> const& query::from() const noexcept {
    return from_;
}

unique_object_ptr<scalar::expression>& query::where() noexcept {
    return where_;
}

unique_object_ptr<scalar::expression> const& query::where() const noexcept {
    return where_;
}

std::optional<group_by_clause>& query::group_by() noexcept {
    return group_by_;
}

std::optional<group_by_clause> const& query::group_by() const noexcept {
    return group_by_;
}

unique_object_ptr<scalar::expression>& query::having() noexcept {
    return having_;
}

unique_object_ptr<scalar::expression> const& query::having() const noexcept {
    return having_;
}

common::vector<common::sort_element>& query::order_by() noexcept {
    return order_by_;
}

common::vector<common::sort_element> const& query::order_by() const noexcept {
    return order_by_;
}

unique_object_ptr<scalar::expression>& query::limit() noexcept {
    return limit_;
}

unique_object_ptr<scalar::expression> const& query::limit() const noexcept {
    return limit_;
}

std::optional<query::quantifier_type>& query::quantifier() noexcept {
    return quantifier_;
}

std::optional<query::quantifier_type> const& query::quantifier() const noexcept {
    return quantifier_;
}

} // namespace mizugaki::ast::query
