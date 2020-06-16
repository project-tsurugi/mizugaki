#include <mizugaki/ast/query/expression.h>

namespace mizugaki::ast::query {

bool operator==(expression const& a, expression const& b) noexcept {
    return a.equals(b);
}

bool operator!=(expression const& a, expression const& b) noexcept {
    return !(a == b);
}

} // namespace mizugaki::ast::query
