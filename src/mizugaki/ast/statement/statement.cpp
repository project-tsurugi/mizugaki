#include <mizugaki/ast/statement/statement.h>

namespace mizugaki::ast::statement {

bool operator==(statement const& a, statement const& b) noexcept {
    return std::addressof(a) == std::addressof(b) || a.equals(b);
}

bool operator!=(statement const& a, statement const& b) noexcept {
    return !(a == b);
}

} // namespace mizugaki::ast::statement
