#pragma once

#include <takatori/util/callback.h>
#include <takatori/util/downcast.h>
#include <takatori/util/fail.h>

#include <shakujo/model/type/ArrayType.h>
#include <shakujo/model/type/BinaryType.h>
#include <shakujo/model/type/BooleanType.h>
#include <shakujo/model/type/CharType.h>
#include <shakujo/model/type/DateType.h>
#include <shakujo/model/type/DecimalType.h>
#include <shakujo/model/type/Float32Type.h>
#include <shakujo/model/type/Float64Type.h>
#include <shakujo/model/type/Int32Type.h>
#include <shakujo/model/type/Int64Type.h>
#include <shakujo/model/type/NullType.h>
#include <shakujo/model/type/RelationType.h>
#include <shakujo/model/type/StringType.h>
#include <shakujo/model/type/TimeType.h>
#include <shakujo/model/type/TimestampType.h>
#include <shakujo/model/type/TupleType.h>
#include <shakujo/model/type/TypeKind.h>
#include <shakujo/model/type/VarBinaryType.h>
#include <shakujo/model/type/VarCharType.h>
#include <shakujo/model/type/VectorType.h>

namespace mizugaki::translator::util {

template<class Callback, class... Args>
auto dispatch(Callback&& callback, ::shakujo::model::type::Type const& node, Args&&... args) {
    using namespace ::shakujo::model::type;
    using ::takatori::util::unsafe_downcast;
    using ::takatori::util::polymorphic_callback;
    switch (node.kind()) {
        case ArrayType::tag: return polymorphic_callback<ArrayType>(std::forward<Callback>(callback), unsafe_downcast<ArrayType>(node), std::forward<Args>(args)...);
        case BinaryType::tag: return polymorphic_callback<BinaryType>(std::forward<Callback>(callback), unsafe_downcast<BinaryType>(node), std::forward<Args>(args)...);
        case BooleanType::tag: return polymorphic_callback<BooleanType>(std::forward<Callback>(callback), unsafe_downcast<BooleanType>(node), std::forward<Args>(args)...);
        case CharType::tag: return polymorphic_callback<CharType>(std::forward<Callback>(callback), unsafe_downcast<CharType>(node), std::forward<Args>(args)...);
        case DateType::tag: return polymorphic_callback<DateType>(std::forward<Callback>(callback), unsafe_downcast<DateType>(node), std::forward<Args>(args)...);
        case DecimalType::tag: return polymorphic_callback<DecimalType>(std::forward<Callback>(callback), unsafe_downcast<DecimalType>(node), std::forward<Args>(args)...);
        case Float32Type::tag: return polymorphic_callback<Float32Type>(std::forward<Callback>(callback), unsafe_downcast<Float32Type>(node), std::forward<Args>(args)...);
        case Float64Type::tag: return polymorphic_callback<Float64Type>(std::forward<Callback>(callback), unsafe_downcast<Float64Type>(node), std::forward<Args>(args)...);
        case Int32Type::tag: return polymorphic_callback<Int32Type>(std::forward<Callback>(callback), unsafe_downcast<Int32Type>(node), std::forward<Args>(args)...);
        case Int64Type::tag: return polymorphic_callback<Int64Type>(std::forward<Callback>(callback), unsafe_downcast<Int64Type>(node), std::forward<Args>(args)...);
        case NullType::tag: return polymorphic_callback<NullType>(std::forward<Callback>(callback), unsafe_downcast<NullType>(node), std::forward<Args>(args)...);
        case RelationType::tag: return polymorphic_callback<RelationType>(std::forward<Callback>(callback), unsafe_downcast<RelationType>(node), std::forward<Args>(args)...);
        case StringType::tag: return polymorphic_callback<StringType>(std::forward<Callback>(callback), unsafe_downcast<StringType>(node), std::forward<Args>(args)...);
        case TimeType::tag: return polymorphic_callback<TimeType>(std::forward<Callback>(callback), unsafe_downcast<TimeType>(node), std::forward<Args>(args)...);
        case TimestampType::tag: return polymorphic_callback<TimestampType>(std::forward<Callback>(callback), unsafe_downcast<TimestampType>(node), std::forward<Args>(args)...);
        case TupleType::tag: return polymorphic_callback<TupleType>(std::forward<Callback>(callback), unsafe_downcast<TupleType>(node), std::forward<Args>(args)...);
        case VarBinaryType::tag: return polymorphic_callback<VarBinaryType>(std::forward<Callback>(callback), unsafe_downcast<VarBinaryType>(node), std::forward<Args>(args)...);
        case VarCharType::tag: return polymorphic_callback<VarCharType>(std::forward<Callback>(callback), unsafe_downcast<VarCharType>(node), std::forward<Args>(args)...);
        case VectorType::tag: return polymorphic_callback<VectorType>(std::forward<Callback>(callback), unsafe_downcast<VectorType>(node), std::forward<Args>(args)...);
    }
    ::takatori::util::fail();
}

} // namespace mizugaki::translator::util
