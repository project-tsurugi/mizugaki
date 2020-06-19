#include <mizugaki/ast/query/grouping_element.h>

#include <mizugaki/ast/common/serializers.h>

namespace mizugaki::ast::query {

bool operator==(grouping_element const& a, grouping_element const& b) noexcept {
    return a.equals(b);
}

bool operator!=(grouping_element const& a, grouping_element const& b) noexcept {
    return !(a == b);
}

std::ostream& operator<<(std::ostream& out, grouping_element const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::query
