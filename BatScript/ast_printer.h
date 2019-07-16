#pragma once

#include <iostream>
#include "ast.h"

namespace Bat
{
	class AstPrinter : public AstVisitor
	{
	public:
		static void Print( AstNode* root );
	private:
		AstPrinter( AstNode* root );
		void PrintNode( AstNode* node );

		void PushLevel();
		void PopLevel();

		int level = -1;
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
	};
}