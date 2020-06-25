#include <mizugaki/ast/statement/constraint.h>

#include <mizugaki/ast/common/serializers.h>

namespace mizugaki::ast::statement {

bool operator==(constraint const& a, constraint const& b) noexcept {
    return std::addressof(a) == std::addressof(b) || a.equals(b);
}

bool operator!=(constraint const& a, constraint const& b) noexcept {
    return !(a == b);
}

std::ostream& operator<<(std::ostream& out, constraint const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::statement
