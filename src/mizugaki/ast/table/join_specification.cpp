#include <mizugaki/ast/table/join_specification.h>

#include <mizugaki/ast/common/serializers.h>

namespace mizugaki::ast::table {

bool operator==(join_specification const& a, join_specification const& b) noexcept {
    return a.equals(b);
}

bool operator!=(join_specification const& a, join_specification const& b) noexcept {
    return !(a == b);
}

std::ostream& operator<<(std::ostream& out, join_specification const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::table
