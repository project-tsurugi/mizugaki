#include <mizugaki/ast/statement/table_element.h>

#include <mizugaki/ast/common/serializers.h>

namespace mizugaki::ast::statement {

bool operator==(table_element const& a, table_element const& b) noexcept {
    return std::addressof(a) == std::addressof(b) || a.equals(b);
}

bool operator!=(table_element const& a, table_element const& b) noexcept {
    return !(a == b);
}

std::ostream& operator<<(std::ostream& out, table_element const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::statement
