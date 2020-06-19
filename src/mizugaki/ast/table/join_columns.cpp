#include <mizugaki/ast/table/join_columns.h>

#include <takatori/util/clonable.h>

#include <mizugaki/ast/common/serializers.h>

#include <mizugaki/ast/compare_utils.h>

namespace mizugaki::ast::table {

using ::takatori::util::object_creator;
using ::takatori::util::unique_object_ptr;

using common::clone_vector;

join_columns::join_columns(
        common::vector<::takatori::util::unique_object_ptr<name::simple>> columns,
        region_type region) noexcept :
    super { region },
    columns_ { std::move(columns) }
{}

join_columns::join_columns(join_columns const& other, object_creator creator) :
    join_columns {
            clone_vector(other.columns_, creator),
            other.region(),
    }
{}

join_columns::join_columns(join_columns&& other, object_creator creator) :
    join_columns {
            clone_vector(std::move(other.columns_), creator),
            other.region(),
    }
{}

join_columns* join_columns::clone(object_creator creator) const& {
    return creator.create_object<join_columns>(*this, creator);
}

join_columns* join_columns::clone(object_creator creator)&& {
    return creator.create_object<join_columns>(std::move(*this), creator);
}

join_specification::node_kind_type join_columns::node_kind() const noexcept {
    return tag;
}

common::vector<unique_object_ptr<name::simple>>& join_columns::columns() noexcept {
    return columns_;
}

common::vector<unique_object_ptr<name::simple>> const& join_columns::columns() const noexcept {
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
