#pragma once

#include "ast.h"
#include "symbol_table.h"

namespace Bat
{
	class SemanticAnalysis : public AstVisitor
	{
	public:
		SemanticAnalysis();
		~SemanticAnalysis();

		void Analyze( Statement* s );
	private:
		void Analyze( Expression* e );
		Type* GetExprType( Expression* e );

		void Error( const SourceLoc& loc, const std::string& message );

		void PushScope();
		void PopScope();

		void AddVariable( AstNode* node, const std::string& name, Type* type );
		void AddFunction( AstNode* node, const std::string& name );
		void AddNative( NativeStmt* node, const std::string& name );
		void AddType( AstNode* node, const std::string& name );
		Type* TokenToType( const Token& tok );
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
		virtual void VisitImportStmt( ImportStmt* node ) override;
		virtual void VisitNativeStmt( NativeStmt* node ) override;
		virtual void VisitVarDecl( VarDecl* node ) override;
		virtual void VisitFuncDecl( FuncDecl* node ) override;
	private:
		Type* m_pResult = nullptr;
		FuncDecl* m_pCurrentFunc = nullptr;
		SymbolTable* m_pSymTab = nullptr;
		std::vector<std::unique_ptr<Statement>> m_pStatements;
	};
}