#include <mizugaki/ast/literal/literal.h>

#include <mizugaki/ast/common/serializers.h>

namespace mizugaki::ast::literal {

bool operator==(literal const& a, literal const& b) noexcept {
    return a.equals(b);
}

bool operator!=(literal const& a, literal const& b) noexcept {
    return !(a == b);
}

void literal::serialize(takatori::serializer::object_acceptor& acceptor) const {
    using namespace common::serializers;
    auto obj = struct_block(acceptor, *this);
    region_property(acceptor, *this);
}

std::ostream& operator<<(std::ostream& out, literal const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast::literal
