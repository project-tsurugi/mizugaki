#include <mizugaki/ast/common/serializers.h>

namespace mizugaki::ast::common::serializers {

::takatori::util::finalizer struct_block(takatori::serializer::object_acceptor& acceptor) {
    acceptor.struct_begin();
    return ::takatori::util::finalizer {
            [&] {
                acceptor.struct_end();
            },
    };
}

::takatori::util::finalizer array_block(takatori::serializer::object_acceptor& acceptor) {
    acceptor.array_begin();
    return ::takatori::util::finalizer {
            [&] {
                acceptor.array_end();
            },
    };
}

::takatori::util::finalizer property_block(takatori::serializer::object_acceptor& acceptor, std::string_view name) {
    acceptor.property_begin(name);
    return ::takatori::util::finalizer {
            [&] {
                acceptor.property_end();
            },
    };
}

} // namespace mizugaki::ast::common::serializers
