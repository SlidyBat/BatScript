#pragma once

#include <stack>
#include "ast.h"
#include "bat_object.h"
#include "environment.h"

namespace Bat
{
	class Interpreter : public AstVisitor
	{
	public:
		Interpreter() = default;

		BatObject Evaluate( Expression* e );
		void Execute( Statement* s );
	private:
		virtual void VisitIntLiteral( IntLiteral* node );
		virtual void VisitFloatLiteral( FloatLiteral* node );
		virtual void VisitStringLiteral( StringLiteral* node );
		virtual void VisitTokenLiteral( TokenLiteral* node );
		virtual void VisitBinaryExpr( BinaryExpr* node );
		virtual void VisitUnaryExpr( UnaryExpr* node );
		virtual void VisitGroupExpr( GroupExpr* node );
		virtual void VisitVarExpr( VarExpr* node );
		virtual void VisitExpressionStmt( ExpressionStmt* node );
		virtual void VisitBlockStmt( BlockStmt* node );
		virtual void VisitPrintStmt( PrintStmt* node );
		virtual void VisitIfStmt( IfStmt* node );
		virtual void VisitWhileStmt( WhileStmt* node );
		virtual void VisitForStmt( ForStmt* node );
		virtual void VisitVarDecl( VarDecl* node );
	private:
		BatObject m_Result;
		Environment m_Environment;
	};
}