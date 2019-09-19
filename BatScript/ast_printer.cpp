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

	void AstPrinter::VisitArrayLiteral( ArrayLiteral* node )
	{
		std::cout << "[]";
		for( size_t i = 0; i < node->NumValues(); i++ )
		{
			PrintNode( node->ValueAt( i ) );
		}
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

	void AstPrinter::VisitCallExpr( CallExpr* node )
	{
		std::cout << "call";
		PrintNode( node->Function() );
		for( size_t i = 0; i < node->NumArgs(); i++ )
		{
			PrintNode( node->Arg( i ) );
		}
	}

	void AstPrinter::VisitIndexExpr( IndexExpr* node )
	{
		std::cout << "index";
		PrintNode( node->Array() );
		PrintNode( node->Index() );
	}

	void AstPrinter::VisitCastExpr( CastExpr* node )
	{
		std::cout << "cast (" << node->TargetType()->ToString() << ")";
		PrintNode( node->Expr() );
	}

	void AstPrinter::VisitGroupExpr( GroupExpr* node )
	{
		std::cout << "()";
		PrintNode( node->Expr() );
	}

	void AstPrinter::VisitVarExpr( VarExpr* node )
	{
		std::cout << "var " << node->Identifier().lexeme;
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

	void AstPrinter::VisitWhileStmt( WhileStmt* node )
	{
		std::cout << "while";
		PrintNode( node->Condition() );
		PrintNode( node->Body() );
	}

	void AstPrinter::VisitForStmt( ForStmt* node )
	{
		std::cout << "for";
		PrintNode( node->Initializer() );
		PrintNode( node->Condition() );
		PrintNode( node->Increment() );
		PrintNode( node->Body() );
	}

	void AstPrinter::VisitReturnStmt( ReturnStmt* node )
	{
		std::cout << "return";
		PrintNode( node->RetExpr() );
	}

	void AstPrinter::VisitImportStmt( ImportStmt* node )
	{
		std::cout << "import " << node->ModuleName();
	}

	void AstPrinter::VisitNativeStmt( NativeStmt* node )
	{
		const auto& sig = node->Signature();
		std::cout << "native " << node->Signature().Identifier().lexeme << std::endl;
		for( size_t i = 0; i < sig.NumParams(); i++ )
		{
			std::cout << sig.ParamType( i ).TypeName().lexeme << std::endl;
			std::cout << sig.ParamIdent( i ).lexeme << std::endl;
			if( sig.ParamDefault( i ) )
			{
				PrintNode( sig.ParamDefault( i ) );
			}
		}
	}

	void AstPrinter::VisitVarDecl( VarDecl* node )
	{
		std::cout << "var " << node->Identifier().lexeme;
		if( node->Initializer() ) PrintNode( node->Initializer() );
	}

	void  AstPrinter::VisitFuncDecl( FuncDecl* node )
	{
		const auto& sig = node->Signature();
		std::cout << "def " << node->Signature().Identifier().lexeme << std::endl;
		for( size_t i = 0; i < sig.NumParams(); i++ )
		{
			std::cout << sig.ParamType( i ).TypeName().lexeme << std::endl;
			std::cout << sig.ParamIdent( i ).lexeme << std::endl;
			if( sig.ParamDefault( i ) )
			{
				PrintNode( sig.ParamDefault( i ) );
			}
		}
		PrintNode( node->Body() );
	}
}