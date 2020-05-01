#pragma once

#include <takatori/util/callback.h>
#include <takatori/util/downcast.h>
#include <takatori/util/fail.h>

#include <shakujo/common/core/type/Visitor.h>

namespace mizugaki::translator::util {

template<class Callback, class... Args>
auto dispatch(Callback&& callback, ::shakujo::common::core::Type const& node, Args&&... args) {
    using namespace ::shakujo::common::core::type;
    using ::takatori::util::unsafe_downcast;
    using ::takatori::util::polymorphic_callback;
    switch (node.kind()) {
        case Int::tag: return polymorphic_callback<Int>(std::forward<Callback>(callback), unsafe_downcast<Int>(node), std::forward<Args>(args)...);
        case Float::tag: return polymorphic_callback<Float>(std::forward<Callback>(callback), unsafe_downcast<Float>(node), std::forward<Args>(args)...);
        case Char::tag: return polymorphic_callback<Char>(std::forward<Callback>(callback), unsafe_downcast<Char>(node), std::forward<Args>(args)...);
        case String::tag: return polymorphic_callback<String>(std::forward<Callback>(callback), unsafe_downcast<String>(node), std::forward<Args>(args)...);
        case Bool::tag: return polymorphic_callback<Bool>(std::forward<Callback>(callback), unsafe_downcast<Bool>(node), std::forward<Args>(args)...);
        case Null::tag: return polymorphic_callback<Null>(std::forward<Callback>(callback), unsafe_downcast<Null>(node), std::forward<Args>(args)...);
        case Tuple::tag: return polymorphic_callback<Tuple>(std::forward<Callback>(callback), unsafe_downcast<Tuple>(node), std::forward<Args>(args)...);
        case Array::tag: return polymorphic_callback<Array>(std::forward<Callback>(callback), unsafe_downcast<Array>(node), std::forward<Args>(args)...);
        case Vector::tag: return polymorphic_callback<Vector>(std::forward<Callback>(callback), unsafe_downcast<Vector>(node), std::forward<Args>(args)...);
        case Relation::tag: return polymorphic_callback<Relation>(std::forward<Callback>(callback), unsafe_downcast<Relation>(node), std::forward<Args>(args)...);
        case Cursor::tag: return polymorphic_callback<Cursor>(std::forward<Callback>(callback), unsafe_downcast<Cursor>(node), std::forward<Args>(args)...);
        case Error::tag: return polymorphic_callback<Error>(std::forward<Callback>(callback), unsafe_downcast<Error>(node), std::forward<Args>(args)...);
    }
    ::takatori::util::fail();
}

} // namespace mizugaki::translator::util
