#include <mizugaki/ast/query/select_element.h>

#include <mizugaki/ast/common/serializers.h>

namespace mizugaki::ast::query {

bool operator==(select_element const& a, select_element const& b) noexcept {
    return a.equals(b);
}

bool operator!=(select_element const& a, select_element const& b) noexcept {
    return !(a == b);
}

std::ostream& operator<<(std::ostream& out, select_element const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::query
