#include <mizugaki/ast/statement/statement.h>

#include <mizugaki/ast/common/serializers.h>

namespace mizugaki::ast::statement {

bool operator==(statement const& a, statement const& b) noexcept {
    return std::addressof(a) == std::addressof(b) || a.equals(b);
}

bool operator!=(statement const& a, statement const& b) noexcept {
    return !(a == b);
}

std::ostream& operator<<(std::ostream& out, statement const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::statement
