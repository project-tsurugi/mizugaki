#include <mizugaki/ast/query/grouping_column.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>
#include <mizugaki/ast/scalar/variable_reference.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::query {

using ::takatori::util::clone_unique;
using ::takatori::util::rvalue_ptr;

grouping_column::grouping_column(
        std::unique_ptr<scalar::expression> column,
        std::unique_ptr<name::name> collation,
        region_type region) noexcept :
    super { region },
    column_ { std::move(column) },
    collation_ { std::move(collation) }
{}

grouping_column::grouping_column(
        scalar::expression&& column,
        rvalue_ptr<name::name> collation,
        region_type region) :
    grouping_column {
            clone_unique(std::move(column)),
            clone_unique(collation),
            region,
    }
{}

static inline std::unique_ptr<scalar::variable_reference> to_expr(name::name&& column) {
    auto r = column.region();
    return std::make_unique<scalar::variable_reference>(std::move(column), r);
}

grouping_column::grouping_column(
        name::name&& column,
        rvalue_ptr<name::name> collation,
        element::region_type region) :
    grouping_column {
            to_expr(std::move(column)),
            clone_unique(collation),
            region,
    }
{}

grouping_column::grouping_column(::takatori::util::clone_tag_t, grouping_column const& other) :
    grouping_column {
            clone_unique(other.column_),
            clone_unique(other.collation_),
            other.region(),
    }
{}

grouping_column::grouping_column(::takatori::util::clone_tag_t, grouping_column&& other) :
    grouping_column {
            clone_unique(std::move(other.column_)),
            clone_unique(std::move(other.collation_)),
            other.region(),
    }
{}

grouping_column* grouping_column::clone() const& {
    return new grouping_column(::takatori::util::clone_tag, *this); // NOLINT
}

grouping_column* grouping_column::clone()&& {
    return new grouping_column(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

grouping_element::node_kind_type grouping_column::node_kind() const noexcept {
    return tag;
}

std::unique_ptr<scalar::expression>& grouping_column::column() noexcept {
    return column_;
}

std::unique_ptr<scalar::expression> const& grouping_column::column() const noexcept {
    return column_;
}

std::unique_ptr<name::name>& grouping_column::collation() noexcept {
    return collation_;
}

std::unique_ptr<name::name> const& grouping_column::collation() const noexcept {
    return collation_;
}

bool operator==(grouping_column const& a, grouping_column const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return false;
    }
    return eq(a.column_, b.column_)
            && eq(a.collation_, b.collation_);
}

bool operator!=(grouping_column const& a, grouping_column const& b) noexcept {
    return !(a == b);
}

bool grouping_column::equals(grouping_element const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void grouping_column::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "column"sv, column_);
    property(acceptor, "collation"sv, collation_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, grouping_column const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::query
