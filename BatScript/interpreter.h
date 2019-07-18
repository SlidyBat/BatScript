#pragma once

#include <stack>
#include "ast.h"
#include "bat_object.h"
#include "bat_callable.h"
#include "environment.h"

namespace Bat
{
	class Interpreter : public AstVisitor
	{
	public:
		Interpreter();
		~Interpreter();

		BatObject Evaluate( Expression* e );
		void Execute( Statement* s );
		void ExecuteBlock( Statement* s, Environment& environment );

		void AddNative( const std::string& name, size_t arity, BatNativeCallback callback );
		Environment* GetEnvironment() { return m_pEnvironment; }
		const Environment* GetEnvironment() const { return m_pEnvironment; }
	private:
		virtual void VisitIntLiteral( IntLiteral* node ) override;
		virtual void VisitFloatLiteral( FloatLiteral* node ) override;
		virtual void VisitStringLiteral( StringLiteral* node ) override;
		virtual void VisitTokenLiteral( TokenLiteral* node ) override;
		virtual void VisitBinaryExpr( BinaryExpr* node ) override;
		virtual void VisitUnaryExpr( UnaryExpr* node ) override;
		virtual void VisitCallExpr( CallExpr* node ) override;
		virtual void VisitGroupExpr( GroupExpr* node ) override;
		virtual void VisitVarExpr( VarExpr* node ) override;
		virtual void VisitExpressionStmt( ExpressionStmt* node ) override;
		virtual void VisitBlockStmt( BlockStmt* node ) override;
		virtual void VisitPrintStmt( PrintStmt* node ) override;
		virtual void VisitIfStmt( IfStmt* node ) override;
		virtual void VisitWhileStmt( WhileStmt* node ) override;
		virtual void VisitForStmt( ForStmt* node ) override;
		virtual void VisitReturnStmt( ReturnStmt* node ) override;
		virtual void VisitVarDecl( VarDecl* node ) override;
		virtual void VisitFuncDecl( FuncDecl* node ) override;
	private:
		BatObject m_Result;
		Environment* m_pEnvironment;
	};
}