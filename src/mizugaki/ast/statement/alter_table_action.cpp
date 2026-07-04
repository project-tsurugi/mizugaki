#include <mizugaki/ast/statement/alter_table_action.h>

#include <mizugaki/ast/common/serializers.h>

namespace mizugaki::ast::statement {

bool operator==(alter_table_action const& a, alter_table_action const& b) noexcept {
    return std::addressof(a) == std::addressof(b) || a.equals(b);
}

bool operator!=(alter_table_action const& a, alter_table_action const& b) noexcept {
    return !(a == b);
}

std::ostream& operator<<(std::ostream& out, alter_table_action const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::statement
