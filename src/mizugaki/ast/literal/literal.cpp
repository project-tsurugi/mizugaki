#include <mizugaki/ast/literal/literal.h>

namespace mizugaki::ast::literal {

bool operator==(literal const& a, literal const& b) noexcept {
    return a.equals(b);
}

bool operator!=(literal const& a, literal const& b) noexcept {
    return !(a == b);
}

} // namespace mizugaki::ast::literal
