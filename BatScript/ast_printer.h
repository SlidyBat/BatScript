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
		virtual void VisitIntLiteral( IntLiteral* node ) override;
		virtual void VisitFloatLiteral( FloatLiteral* node ) override;
		virtual void VisitStringLiteral( StringLiteral* node ) override;
		virtual void VisitTokenLiteral( TokenLiteral* node ) override;
		virtual void VisitArrayLiteral( ArrayLiteral* node ) override;
		virtual void VisitBinaryExpr( BinaryExpr* node ) override;
		virtual void VisitUnaryExpr( UnaryExpr* node ) override;
		virtual void VisitCallExpr( CallExpr* node ) override;
		virtual void VisitIndexExpr( IndexExpr* node ) override;
		virtual void VisitCastExpr( CastExpr* node ) override;
		virtual void VisitGroupExpr( GroupExpr* node ) override;
		virtual void VisitVarExpr( VarExpr* node ) override;
		virtual void VisitExpressionStmt( ExpressionStmt* node ) override;
		virtual void VisitAssignStmt( AssignStmt* node ) override;
		virtual void VisitBlockStmt( BlockStmt* node ) override;
		virtual void VisitPrintStmt( PrintStmt* node ) override;
		virtual void VisitIfStmt( IfStmt* node ) override;
		virtual void VisitWhileStmt( WhileStmt* node ) override;
		virtual void VisitForStmt( ForStmt* node ) override;
		virtual void VisitReturnStmt( ReturnStmt* node ) override;
		virtual void VisitImportStmt( ImportStmt* node ) override;
		virtual void VisitNativeStmt( NativeStmt* node ) override;
		virtual void VisitVarDecl( VarDecl* node ) override;
		virtual void VisitFuncDecl( FuncDecl* node ) override;
	};
}