#include <mizugaki/ast/node.h>

#include <mizugaki/ast/common/serializers.h>

namespace mizugaki::ast {

using ::takatori::serializer::object_acceptor;

object_acceptor& operator<<(object_acceptor& acceptor, node const& value) {
    value.serialize(acceptor);
    return acceptor;
}

std::ostream& operator<<(std::ostream& out, node const& value) {
    return common::serializers::print(out, value);
}

} // namespace mizugaki::ast
