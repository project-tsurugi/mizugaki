#include <mizugaki/ast/table/expression.h>

namespace mizugaki::ast::table {

bool operator==(expression const& a, expression const& b) noexcept {
    return a.equals(b);
}

bool operator!=(expression const& a, expression const& b) noexcept {
    return !(a == b);
}

} // namespace mizugaki::ast::table
