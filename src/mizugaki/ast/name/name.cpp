#include <mizugaki/ast/name/name.h>

namespace mizugaki::ast::name {

bool operator==(name const& a, name const& b) noexcept {
    return std::addressof(a) == std::addressof(b) || a.equals(b);
}

bool operator!=(name const& a, name const& b) noexcept {
    return !(a == b);
}

} // namespace mizugaki::ast::name
