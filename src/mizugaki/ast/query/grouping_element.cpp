#include <mizugaki/ast/query/grouping_element.h>

namespace mizugaki::ast::query {

bool operator==(grouping_element const& a, grouping_element const& b) noexcept {
    return a.equals(b);
}

bool operator!=(grouping_element const& a, grouping_element const& b) noexcept {
    return !(a == b);
}

} // namespace mizugaki::ast::query
