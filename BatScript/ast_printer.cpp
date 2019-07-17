#include "ast_printer.h"

namespace Bat
{

	void AstPrinter::Print( AstNode* root )
	{
		AstPrinter printer( root );
	}

	AstPrinter::AstPrinter( AstNode* root )
	{
		PrintNode( root );
		std::cout << std::endl;
	}

	void AstPrinter::PrintNode( AstNode* node )
	{
		PushLevel();
		node->Accept( this );
		PopLevel();
	}

	void AstPrinter::PushLevel()
	{
		level++;
		std::cout << std::endl;
		for( int i = 0; i < level; i++ )
		{
			std::cout << "  ";
		}
	}

	void AstPrinter::PopLevel()
	{
		level--;
	}

	void AstPrinter::VisitIntLiteral( IntLiteral* node )
	{
		std::cout << node->value;
	}

	void AstPrinter::VisitFloatLiteral( FloatLiteral* node )
	{
		std::cout << node->value;
	}

	void AstPrinter::VisitStringLiteral( StringLiteral* node )
	{
		std::cout << node->value;
	}

	void AstPrinter::VisitTokenLiteral( TokenLiteral* node )
	{
		std::cout << TokenTypeToString( node->value );
	}

	void AstPrinter::VisitBinaryExpr( BinaryExpr* node )
	{
		std::cout << TokenTypeToString( node->Op() );
		PrintNode( node->Left() );
		PrintNode( node->Right() );
	}

	void AstPrinter::VisitUnaryExpr( UnaryExpr* node )
	{
		std::cout << TokenTypeToString( node->Op() );
		PrintNode( node->Right() );
	}

	void AstPrinter::VisitGroupExpr( GroupExpr* node )
	{
		std::cout << "()";
		PrintNode( node->Expr() );
	}

	void AstPrinter::VisitVarExpr( VarExpr* node )
	{
		std::cout << "var " << node->name.lexeme;
	}

	void AstPrinter::VisitExpressionStmt( ExpressionStmt* node )
	{
		std::cout << "expr";
		PrintNode( node->Expr() );
	}

	void AstPrinter::VisitBlockStmt( BlockStmt* node )
	{
		std::cout << "{}";
		for( size_t i = 0; i < node->NumStatements(); i++ )
		{
			PrintNode( node->Stmt( i ) );
		}
	}

	void AstPrinter::VisitPrintStmt( PrintStmt* node )
	{
		std::cout << "print";
		PrintNode( node->Expr() );
	}

	void AstPrinter::VisitIfStmt( IfStmt* node )
	{
		std::cout << "if";
		PrintNode( node->Then() );
		if( node->Else() )
		{
			std::cout << "else";
			PrintNode( node->Else() );
		}
	}

	void AstPrinter::VisitVarDecl( VarDecl* node )
	{
		std::cout << "var " << node->Identifier().lexeme;
		if( node->Initializer() ) PrintNode( node->Initializer() );
	}
}