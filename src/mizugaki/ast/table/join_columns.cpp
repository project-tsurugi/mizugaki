#include <mizugaki/ast/table/join_columns.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::table {


using common::clone_vector;
using common::to_vector;

join_columns::join_columns(
        std::vector<std::unique_ptr<name::simple>> columns,
        region_type region) noexcept :
    super { region },
    columns_ { std::move(columns) }
{}

join_columns::join_columns(
        common::rvalue_list<name::simple> columns,
        region_type region) :
    join_columns {
            to_vector(columns),
            region,
    }
{}

join_columns::join_columns(::takatori::util::clone_tag_t, join_columns const& other) :
    join_columns {
            clone_vector(other.columns_),
            other.region(),
    }
{}

join_columns::join_columns(::takatori::util::clone_tag_t, join_columns&& other) :
    join_columns {
            clone_vector(std::move(other.columns_)),
            other.region(),
    }
{}

join_columns* join_columns::clone() const& {
    return new join_columns(::takatori::util::clone_tag, *this); // NOLINT
}

join_columns* join_columns::clone()&& {
    return new join_columns(::takatori::util::clone_tag, std::move(*this)); // NOLINT;
}

join_specification::node_kind_type join_columns::node_kind() const noexcept {
    return tag;
}

std::vector<std::unique_ptr<name::simple>>& join_columns::columns() noexcept {
    return columns_;
}

std::vector<std::unique_ptr<name::simple>> const& join_columns::columns() const noexcept {
    return columns_;
}

bool operator==(join_columns const& a, join_columns const& b) noexcept {
    if (std::addressof(a) == std::addressof(b)) {
        return false;
    }
    return eq(a.columns_, b.columns_);
}

bool operator!=(join_columns const& a, join_columns const& b) noexcept {
    return !(a == b);
}

bool join_columns::equals(join_specification const& other) const noexcept {
    return other.node_kind() == tag
            && *this == unsafe_downcast<type_of_t<tag>>(other);
}

void join_columns::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    auto obj = struct_block(acceptor, *this);
    property(acceptor, "columns"sv, columns_);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, join_columns const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::table
