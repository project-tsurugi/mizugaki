#include <mizugaki/ast/statement/alter_index_action.h>

#include <mizugaki/ast/common/serializers.h>

namespace mizugaki::ast::statement {

bool operator==(alter_index_action const& a, alter_index_action const& b) noexcept {
    return std::addressof(a) == std::addressof(b) || a.equals(b);
}

bool operator!=(alter_index_action const& a, alter_index_action const& b) noexcept {
    return !(a == b);
}

std::ostream& operator<<(std::ostream& out, alter_index_action const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::statement
