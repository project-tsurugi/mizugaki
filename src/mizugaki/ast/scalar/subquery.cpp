#include <mizugaki/ast/scalar/subquery.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::scalar {

using ::takatori::util::clone_unique;

subquery::subquery(
        std::unique_ptr<ast::query::expression> query,
        context_kind_type context_kind,
        region_type region) noexcept :
    super { region },
    query_ { std::move(query) },
    context_kind_ { context_kind }
{}

subquery::subquery(
        ast::query::expression&& query,
        context_kind_type context_kind,
        region_type region) noexcept :
    subquery {
            clone_unique(std::move(query)),
            context_kind,
            region,
    }
{}

subquery::subquery(::takatori::util::clone_tag_t, subquery const& other) :
    subquery {
            clone_unique(other.query_),
            other.context_kind_,
            other.region(),
    }
{}

subquery::subquery(::takatori::util::clone_tag_t, subquery&& other) :
    subquery {
            clone_unique(std::move(other.query_)),
            other.context_kind_,
            other.region(),
    }
{}

subquery* subquery::clone() const& {
    return new subquery(::takatori::util::clone_tag, *this); // NOLINT
}

subquery* subquery::clone() && {
    return new subquery(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

expression::node_kind_type subquery::node_kind() const noexcept {
    return tag;
}

std::unique_ptr<ast::query::expression>& subquery::query() noexcept {
    return query_;
}

std::unique_ptr<ast::query::expression> const& subquery::query() const noexcept {
    return query_;
}

subquery::context_kind_type& subquery::context_kind() noexcept {
    return context_kind_;
}

subquery::context_kind_type const& subquery::context_kind() const noexcept {
    return context_kind_;
}

bool operator==(subquery const& a, subquery const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return false;
    }
    return eq(a.query_, b.query_) && a.context_kind_ == b.context_kind_;
}

bool operator!=(subquery const& a, subquery const& b) noexcept {
    return !(a == b);
}

bool subquery::equals(scalar::expression const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void subquery::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "query"sv, query_);
    property(acceptor, "context_kind"sv, context_kind_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, subquery const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::scalar
