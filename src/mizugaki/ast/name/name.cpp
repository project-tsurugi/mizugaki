#include <mizugaki/ast/name/name.h>

#include <mizugaki/ast/name/simple.h>
#include <mizugaki/ast/common/serializers.h>

namespace mizugaki::ast::name {

const name::identifier_type &name::last_identifier() const noexcept {
    return last_name().identifier();
}

bool operator==(name const& a, name const& b) noexcept {
    return std::addressof(a) == std::addressof(b) || a.equals(b);
}

bool operator!=(name const& a, name const& b) noexcept {
    return !(a == b);
}

std::ostream& operator<<(std::ostream& out, name const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::name
