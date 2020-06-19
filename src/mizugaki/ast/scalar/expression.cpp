#include <mizugaki/ast/scalar/expression.h>

#include <mizugaki/ast/common/serializers.h>

namespace mizugaki::ast::scalar {

bool operator==(expression const& a, expression const& b) noexcept {
    return a.equals(b);
}

bool operator!=(expression const& a, expression const& b) noexcept {
    return !(a == b);
}

std::ostream& operator<<(std::ostream& out, expression const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::scalar
