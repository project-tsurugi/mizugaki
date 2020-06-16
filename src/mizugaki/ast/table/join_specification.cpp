#include <mizugaki/ast/table/join_specification.h>

namespace mizugaki::ast::table {

bool operator==(join_specification const& a, join_specification const& b) noexcept {
    return a.equals(b);
}

bool operator!=(join_specification const& a, join_specification const& b) noexcept {
    return !(a == b);
}

} // namespace mizugaki::ast::table
