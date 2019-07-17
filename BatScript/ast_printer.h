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
		virtual void VisitVarDecl( VarDecl* node );
	};
}