#include <mizugaki/ast/query/select_element.h>

namespace mizugaki::ast::query {

bool operator==(select_element const& a, select_element const& b) noexcept {
    return a.equals(b);
}

bool operator!=(select_element const& a, select_element const& b) noexcept {
    return !(a == b);
}

} // namespace mizugaki::ast::query
