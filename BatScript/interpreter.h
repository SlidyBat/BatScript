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
		virtual void VisitIntLiteral( Bat::IntLiteral* node );
		virtual void VisitFloatLiteral( Bat::FloatLiteral* node );
		virtual void VisitStringLiteral( Bat::StringLiteral* node );
		virtual void VisitTokenLiteral( Bat::TokenLiteral* node );
		virtual void VisitBinaryExpr( Bat::BinaryExpr* node );
		virtual void VisitUnaryExpr( Bat::UnaryExpr* node );
		virtual void VisitGroupExpr( Bat::GroupExpr* node );
		virtual void VisitVarExpr( Bat::VarExpr* node );
		virtual void VisitExpressionStmt( Bat::ExpressionStmt* node );
		virtual void VisitPrintStmt( Bat::PrintStmt* node );
		virtual void VisitVarDecl( Bat::VarDecl* node );
	private:
		BatObject m_Result;
		Environment m_Environment;
	};
}