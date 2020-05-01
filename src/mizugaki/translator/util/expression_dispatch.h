#pragma once

#include <takatori/util/callback.h>
#include <takatori/util/downcast.h>
#include <takatori/util/fail.h>

#include <shakujo/model/expression/ExpressionVisitor.h>

namespace mizugaki::translator::util {

template<class Callback, class... Args>
auto dispatch(Callback&& callback, ::shakujo::model::expression::Expression const& node, Args&&... args) {
    using namespace ::shakujo::model::expression;
    using ::takatori::util::unsafe_downcast;
    using ::takatori::util::polymorphic_callback;
    switch (node.kind()) {
        case ArrayCreationExpression::tag: return polymorphic_callback<ArrayCreationExpression>(std::forward<Callback>(callback), unsafe_downcast<ArrayCreationExpression>(node), std::forward<Args>(args)...);
        case ArrayElementLoadExpression::tag: return polymorphic_callback<ArrayElementLoadExpression>(std::forward<Callback>(callback), unsafe_downcast<ArrayElementLoadExpression>(node), std::forward<Args>(args)...);
        case ArrayElementStoreExpression::tag: return polymorphic_callback<ArrayElementStoreExpression>(std::forward<Callback>(callback), unsafe_downcast<ArrayElementStoreExpression>(node), std::forward<Args>(args)...);
        case AssignExpression::tag: return polymorphic_callback<AssignExpression>(std::forward<Callback>(callback), unsafe_downcast<AssignExpression>(node), std::forward<Args>(args)...);
        case BinaryOperator::tag: return polymorphic_callback<BinaryOperator>(std::forward<Callback>(callback), unsafe_downcast<BinaryOperator>(node), std::forward<Args>(args)...);
        case BlockExpression::tag: return polymorphic_callback<BlockExpression>(std::forward<Callback>(callback), unsafe_downcast<BlockExpression>(node), std::forward<Args>(args)...);
        case CaseExpression::tag: return polymorphic_callback<CaseExpression>(std::forward<Callback>(callback), unsafe_downcast<CaseExpression>(node), std::forward<Args>(args)...);
        case CursorAdvanceExpression::tag: return polymorphic_callback<CursorAdvanceExpression>(std::forward<Callback>(callback), unsafe_downcast<CursorAdvanceExpression>(node), std::forward<Args>(args)...);
        case CursorCreationExpression::tag: return polymorphic_callback<CursorCreationExpression>(std::forward<Callback>(callback), unsafe_downcast<CursorCreationExpression>(node), std::forward<Args>(args)...);
        case CursorElementLoadExpression::tag: return polymorphic_callback<CursorElementLoadExpression>(std::forward<Callback>(callback), unsafe_downcast<CursorElementLoadExpression>(node), std::forward<Args>(args)...);
        case FunctionCall::tag: return polymorphic_callback<FunctionCall>(std::forward<Callback>(callback), unsafe_downcast<FunctionCall>(node), std::forward<Args>(args)...);
        case ImplicitCast::tag: return polymorphic_callback<ImplicitCast>(std::forward<Callback>(callback), unsafe_downcast<ImplicitCast>(node), std::forward<Args>(args)...);
        case Literal::tag: return polymorphic_callback<Literal>(std::forward<Callback>(callback), unsafe_downcast<Literal>(node), std::forward<Args>(args)...);
        case Placeholder::tag: return polymorphic_callback<Placeholder>(std::forward<Callback>(callback), unsafe_downcast<Placeholder>(node), std::forward<Args>(args)...);
        case TupleCreationExpression::tag: return polymorphic_callback<TupleCreationExpression>(std::forward<Callback>(callback), unsafe_downcast<TupleCreationExpression>(node), std::forward<Args>(args)...);
        case TupleElementLoadExpression::tag: return polymorphic_callback<TupleElementLoadExpression>(std::forward<Callback>(callback), unsafe_downcast<TupleElementLoadExpression>(node), std::forward<Args>(args)...);
        case TupleElementStoreExpression::tag: return polymorphic_callback<TupleElementStoreExpression>(std::forward<Callback>(callback), unsafe_downcast<TupleElementStoreExpression>(node), std::forward<Args>(args)...);
        case TypeOperator::tag: return polymorphic_callback<TypeOperator>(std::forward<Callback>(callback), unsafe_downcast<TypeOperator>(node), std::forward<Args>(args)...);
        case UnaryOperator::tag: return polymorphic_callback<UnaryOperator>(std::forward<Callback>(callback), unsafe_downcast<UnaryOperator>(node), std::forward<Args>(args)...);
        case VariableReference::tag: return polymorphic_callback<VariableReference>(std::forward<Callback>(callback), unsafe_downcast<VariableReference>(node), std::forward<Args>(args)...);
        case VectorCreationExpression::tag: return polymorphic_callback<VectorCreationExpression>(std::forward<Callback>(callback), unsafe_downcast<VectorCreationExpression>(node), std::forward<Args>(args)...);
        case VectorElementLoadExpression::tag: return polymorphic_callback<VectorElementLoadExpression>(std::forward<Callback>(callback), unsafe_downcast<VectorElementLoadExpression>(node), std::forward<Args>(args)...);
        case VectorElementStoreExpression::tag: return polymorphic_callback<VectorElementStoreExpression>(std::forward<Callback>(callback), unsafe_downcast<VectorElementStoreExpression>(node), std::forward<Args>(args)...);
        case VectorLengthExpression::tag: return polymorphic_callback<VectorLengthExpression>(std::forward<Callback>(callback), unsafe_downcast<VectorLengthExpression>(node), std::forward<Args>(args)...);
        case relation::AggregationExpression::tag: return polymorphic_callback<relation::AggregationExpression>(std::forward<Callback>(callback), unsafe_downcast<relation::AggregationExpression>(node), std::forward<Args>(args)...);
        case relation::DistinctExpression::tag: return polymorphic_callback<relation::DistinctExpression>(std::forward<Callback>(callback), unsafe_downcast<relation::DistinctExpression>(node), std::forward<Args>(args)...);
        case relation::GroupExpression::tag: return polymorphic_callback<relation::GroupExpression>(std::forward<Callback>(callback), unsafe_downcast<relation::GroupExpression>(node), std::forward<Args>(args)...);
        case relation::JoinExpression::tag: return polymorphic_callback<relation::JoinExpression>(std::forward<Callback>(callback), unsafe_downcast<relation::JoinExpression>(node), std::forward<Args>(args)...);
        case relation::LimitExpression::tag: return polymorphic_callback<relation::LimitExpression>(std::forward<Callback>(callback), unsafe_downcast<relation::LimitExpression>(node), std::forward<Args>(args)...);
        case relation::OrderExpression::tag: return polymorphic_callback<relation::OrderExpression>(std::forward<Callback>(callback), unsafe_downcast<relation::OrderExpression>(node), std::forward<Args>(args)...);
        case relation::ProjectionExpression::tag: return polymorphic_callback<relation::ProjectionExpression>(std::forward<Callback>(callback), unsafe_downcast<relation::ProjectionExpression>(node), std::forward<Args>(args)...);
        case relation::RenameExpression::tag: return polymorphic_callback<relation::RenameExpression>(std::forward<Callback>(callback), unsafe_downcast<relation::RenameExpression>(node), std::forward<Args>(args)...);
        case relation::ScanExpression::tag: return polymorphic_callback<relation::ScanExpression>(std::forward<Callback>(callback), unsafe_downcast<relation::ScanExpression>(node), std::forward<Args>(args)...);
        case relation::SelectionExpression::tag: return polymorphic_callback<relation::SelectionExpression>(std::forward<Callback>(callback), unsafe_downcast<relation::SelectionExpression>(node), std::forward<Args>(args)...);
    }
    ::takatori::util::fail();
}

} // namespace mizugaki::translator::util
