#include <mizugaki/ast/scalar/expression.h>

namespace mizugaki::ast::scalar {

bool operator==(expression const& a, expression const& b) noexcept {
    return a.equals(b);
}

bool operator!=(expression const& a, expression const& b) noexcept {
    return !(a == b);
}

} // namespace mizugaki::ast::scalar
