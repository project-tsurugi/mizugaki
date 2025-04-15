#include <mizugaki/ast/node_region.h>

#include <mizugaki/ast/common/serializers.h>

#include <algorithm>

namespace mizugaki::ast {

node_region operator|(node_region a, node_region b) noexcept {
    if (!a) {
        return b;
    }
    if (!b) {
        return a;
    }
    auto begin = std::min(a.begin, b.begin);
    auto end = std::max(a.end, b.end);
    return { begin, end };
}

bool operator==(node_region a, node_region b) noexcept {
    return a.begin == b.begin && a.end == b.end;
}

bool operator!=(node_region a, node_region b) noexcept {
    return !(a == b);
}

std::ostream& operator<<(std::ostream& out, node_region value) {
    if (value) {
        return out << "region("
                   << "begin=" << value.begin << ", "
                   << "end=" << value.end << ")";
    }
    return out << "region()";
}

::takatori::serializer::object_acceptor& operator<<(::takatori::serializer::object_acceptor& acceptor, node_region value) {
    using namespace common::serializers;
    using namespace std::string_view_literals;
    if (value) {
        auto obj = struct_block(acceptor);
        property(acceptor, "begin"sv, value.begin);
        property(acceptor, "end"sv, value.end);
    }
    return acceptor;
}

} // namespace mizugaki::ast

std::size_t std::hash<::mizugaki::ast::node_region>::operator()(::mizugaki::ast::node_region object) const noexcept {
    if (object) {
        return object.begin * 31 + object.end;
    }
    return {};
}
