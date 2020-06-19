#include <mizugaki/ast/type/type.h>

#include <mizugaki/ast/common/serializers.h>

namespace mizugaki::ast::type {

bool operator==(type const& a, type const& b) noexcept {
    return a.equals(b);
}

bool operator!=(type const& a, type const& b) noexcept {
    return !(a == b);
}

std::ostream& operator<<(std::ostream& out, type const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::type
