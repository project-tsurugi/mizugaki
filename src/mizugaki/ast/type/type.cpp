#include <mizugaki/ast/type/type.h>

namespace mizugaki::ast::type {

bool operator==(type const& a, type const& b) noexcept {
    return a.equals(b);
}

bool operator!=(type const& a, type const& b) noexcept {
    return !(a == b);
}

} // namespace mizugaki::ast::type
