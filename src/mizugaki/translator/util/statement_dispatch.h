#pragma once

#include <takatori/util/callback.h>
#include <takatori/util/downcast.h>
#include <takatori/util/fail.h>

#include <shakujo/model/statement/StatementVisitor.h>

namespace mizugaki::translator::util {

template<class Callback, class... Args>
auto dispatch(Callback&& callback, ::shakujo::model::statement::Statement const& node, Args&&... args) {
    using namespace ::shakujo::model::statement;
    using ::takatori::util::unsafe_downcast;
    using ::takatori::util::polymorphic_callback;
    switch (node.kind()) {
        case AnchorDeclaration::tag: return polymorphic_callback<AnchorDeclaration>(std::forward<Callback>(callback), unsafe_downcast<AnchorDeclaration>(node), std::forward<Args>(args)...);  
        case BlockStatement::tag: return polymorphic_callback<BlockStatement>(std::forward<Callback>(callback), unsafe_downcast<BlockStatement>(node), std::forward<Args>(args)...);  
        case BranchStatement::tag: return polymorphic_callback<BranchStatement>(std::forward<Callback>(callback), unsafe_downcast<BranchStatement>(node), std::forward<Args>(args)...);  
        case BreakStatement::tag: return polymorphic_callback<BreakStatement>(std::forward<Callback>(callback), unsafe_downcast<BreakStatement>(node), std::forward<Args>(args)...);  
        case ContinueStatement::tag: return polymorphic_callback<ContinueStatement>(std::forward<Callback>(callback), unsafe_downcast<ContinueStatement>(node), std::forward<Args>(args)...);  
        case EmptyStatement::tag: return polymorphic_callback<EmptyStatement>(std::forward<Callback>(callback), unsafe_downcast<EmptyStatement>(node), std::forward<Args>(args)...);  
        case ExpressionStatement::tag: return polymorphic_callback<ExpressionStatement>(std::forward<Callback>(callback), unsafe_downcast<ExpressionStatement>(node), std::forward<Args>(args)...);  
        case ForEachStatement::tag: return polymorphic_callback<ForEachStatement>(std::forward<Callback>(callback), unsafe_downcast<ForEachStatement>(node), std::forward<Args>(args)...);  
        case ForStatement::tag: return polymorphic_callback<ForStatement>(std::forward<Callback>(callback), unsafe_downcast<ForStatement>(node), std::forward<Args>(args)...);  
        case LocalVariableDeclaration::tag: return polymorphic_callback<LocalVariableDeclaration>(std::forward<Callback>(callback), unsafe_downcast<LocalVariableDeclaration>(node), std::forward<Args>(args)...);  
        case LogStatement::tag: return polymorphic_callback<LogStatement>(std::forward<Callback>(callback), unsafe_downcast<LogStatement>(node), std::forward<Args>(args)...);  
        case RaiseStatement::tag: return polymorphic_callback<RaiseStatement>(std::forward<Callback>(callback), unsafe_downcast<RaiseStatement>(node), std::forward<Args>(args)...);  
        case ReturnStatement::tag: return polymorphic_callback<ReturnStatement>(std::forward<Callback>(callback), unsafe_downcast<ReturnStatement>(node), std::forward<Args>(args)...);  
        case VectorElementDeleteStatement::tag: return polymorphic_callback<VectorElementDeleteStatement>(std::forward<Callback>(callback), unsafe_downcast<VectorElementDeleteStatement>(node), std::forward<Args>(args)...);  
        case VectorElementInsertStatement::tag: return polymorphic_callback<VectorElementInsertStatement>(std::forward<Callback>(callback), unsafe_downcast<VectorElementInsertStatement>(node), std::forward<Args>(args)...);  
        case WhileStatement::tag: return polymorphic_callback<WhileStatement>(std::forward<Callback>(callback), unsafe_downcast<WhileStatement>(node), std::forward<Args>(args)...);  
        case ddl::CreateTableStatement::tag: return polymorphic_callback<ddl::CreateTableStatement>(std::forward<Callback>(callback), unsafe_downcast<ddl::CreateTableStatement>(node), std::forward<Args>(args)...);  
        case ddl::DropTableStatement::tag: return polymorphic_callback<ddl::DropTableStatement>(std::forward<Callback>(callback), unsafe_downcast<ddl::DropTableStatement>(node), std::forward<Args>(args)...);  
        case dml::DeleteStatement::tag: return polymorphic_callback<dml::DeleteStatement>(std::forward<Callback>(callback), unsafe_downcast<dml::DeleteStatement>(node), std::forward<Args>(args)...);  
        case dml::EmitStatement::tag: return polymorphic_callback<dml::EmitStatement>(std::forward<Callback>(callback), unsafe_downcast<dml::EmitStatement>(node), std::forward<Args>(args)...);  
        case dml::InsertRelationStatement::tag: return polymorphic_callback<dml::InsertRelationStatement>(std::forward<Callback>(callback), unsafe_downcast<dml::InsertRelationStatement>(node), std::forward<Args>(args)...);  
        case dml::InsertValuesStatement::tag: return polymorphic_callback<dml::InsertValuesStatement>(std::forward<Callback>(callback), unsafe_downcast<dml::InsertValuesStatement>(node), std::forward<Args>(args)...);  
        case dml::UpdateStatement::tag: return polymorphic_callback<dml::UpdateStatement>(std::forward<Callback>(callback), unsafe_downcast<dml::UpdateStatement>(node), std::forward<Args>(args)...);  
        case transaction::TransactionBlockStatement::tag: return polymorphic_callback<transaction::TransactionBlockStatement>(std::forward<Callback>(callback), unsafe_downcast<transaction::TransactionBlockStatement>(node), std::forward<Args>(args)...);  
    }
    ::takatori::util::fail();
}

} // namespace mizugaki::translator::util
