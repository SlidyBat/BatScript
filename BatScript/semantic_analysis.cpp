#include "semantic_analysis.h"

#include "type_manager.h"
#include "errorsys.h"

#define BAT_RETURN( value ) do { m_pResult = (value); return; } while( false )

namespace Bat
{
	// Safe way of saving symbol table and restoring at end of scope
	class SymbolTableRestore
	{
	public:
		SymbolTableRestore( SymbolTable** to )
			:
			to( to ),
			value( *to )
		{}
		~SymbolTableRestore()
		{
			*to = value;
		}
	private:
		SymbolTable** to;
		SymbolTable* value;
	};

	static bool IsNumericType( Type* type )
	{
		if( !type ) return false;

		switch( type->Kind() )
		{
			case TypeKind::Array:
			case TypeKind::Function:
				return false;
			case TypeKind::Primitive:
				switch( type->AsPrimitive()->PrimKind() )
				{
					case PrimitiveKind::Bool:
					case PrimitiveKind::String:
						return false;
					case PrimitiveKind::Int:
					case PrimitiveKind::Float:
						return true;
				}
		}

		assert( false );
		return false;
	}

	SemanticAnalysis::SemanticAnalysis()
	{
		m_pSymTab = new SymbolTable;
	}
	SemanticAnalysis::~SemanticAnalysis()
	{
		delete m_pSymTab;
	}
	void SemanticAnalysis::Analyze( Expression* e )
	{
		e->Accept( this );
	}
	void SemanticAnalysis::Analyze( Statement* s )
	{
		s->Accept( this );
	}
	Type* SemanticAnalysis::GetExprType( Expression* e )
	{
		e->Accept( this );
		return m_pResult;
	}
	void SemanticAnalysis::Error( const SourceLoc& loc, const std::string& message )
	{
		ErrorSys::Report( loc.Line(), loc.Column(), message );
	}
	void SemanticAnalysis::PushScope()
	{
		m_pSymTab = new SymbolTable( m_pSymTab );
	}
	void SemanticAnalysis::PopScope()
	{
		assert( m_pSymTab->Enclosing() != nullptr );
		SymbolTable* temp = m_pSymTab;
		m_pSymTab = m_pSymTab->Enclosing();
		delete temp;
	}
	void SemanticAnalysis::AddVariable( AstNode* node, const std::string& name, Type* type )
	{
		m_pSymTab->AddSymbol( name, std::make_unique<VariableSymbol>( node, type ) );
	}
	void SemanticAnalysis::AddFunction( AstNode* node, const std::string& name )
	{
		m_pSymTab->AddSymbol( name, std::make_unique<FunctionSymbol>( node ) );
	}
	Type* SemanticAnalysis::TokenToType( const Token& tok )
	{
		return typeman.GetType( tok.lexeme );
	}
	void SemanticAnalysis::VisitIntLiteral( IntLiteral* node )
	{
		BAT_RETURN( typeman.GetType( PrimitiveKind::Int ) );
	}
	void SemanticAnalysis::VisitFloatLiteral( FloatLiteral* node )
	{
		BAT_RETURN( typeman.GetType( PrimitiveKind::Float ) );
	}
	void SemanticAnalysis::VisitStringLiteral( StringLiteral* node )
	{
		BAT_RETURN( typeman.GetType( PrimitiveKind::String ) );
	}
	void SemanticAnalysis::VisitTokenLiteral( TokenLiteral* node )
	{
		switch( node->value )
		{
			case TOKEN_TRUE:
			case TOKEN_FALSE:
				BAT_RETURN( typeman.GetType( PrimitiveKind::Bool ) );
		}
		assert( false );
		BAT_RETURN( typeman.GetType( PrimitiveKind::Int ) ); // TODO: ???
	}
	void SemanticAnalysis::VisitBinaryExpr( BinaryExpr* node )
	{
		Type* left = GetExprType( node->Left() );
		Type* right = GetExprType( node->Right() );

		if( !IsNumericType( left ) || !IsNumericType( right ) )
		{
			Error( node->Location(), std::string( "Cannot use operator '" ) + TokenTypeToString( node->Op() ) + "' on expressions of types " + left->ToString()
				+ " and " + right->ToString() );
		}

		auto pleft = left->ToPrimitive();
		auto pright = right->ToPrimitive();
		if( pleft->PrimKind() == pright->PrimKind() )
		{
			BAT_RETURN( left );
		}
		if( pleft->PrimKind() == PrimitiveKind::Float )
		{
			BAT_RETURN( pleft );
		}
		if( pright->PrimKind() == PrimitiveKind::Float )
		{
			BAT_RETURN( pright );
		}

		assert( false );
		BAT_RETURN( pleft );
	}
	void SemanticAnalysis::VisitUnaryExpr( UnaryExpr* node )
	{
		Type* right = GetExprType( node->Right() );
		if( !IsNumericType( right ) )
		{
			Error( node->Location(), std::string( "Cannot use operator '" ) + TokenTypeToString( node->Op() ) + "' on expression of type " + right->ToString() );
		}

		BAT_RETURN( right );
	}
	void SemanticAnalysis::VisitCallExpr( CallExpr* node )
	{
		Type* t = GetExprType( node->Function() );
		Symbol* symbol = m_pSymTab->GetSymbol( node->Function()->AsVarExpr()->name.lexeme );
		if( !symbol || !symbol->IsFunction() )
		{
			Error( node->Location(), node->Function()->AsVarExpr()->name.lexeme + " is not a function" );
			BAT_RETURN( t );
		}

		FunctionSymbol* func_symbol = symbol->ToFunction();
		auto& sig = func_symbol->Node()->ToFuncDecl()->Signature();
		Type* ret_type = sig.ReturnType();

		if( node->NumArgs() != sig.NumParams() ) // TODO: handle defaults
		{
			Error( node->Location(), "Expected " + std::to_string( sig.NumParams() ) + " arguments, got " + std::to_string( node->NumArgs() ) );
		}
		for( size_t i = 0; i < sig.NumParams(); i++ )
		{
			Type* expected_type = TokenToType( sig.ParamType( i ) );
			Type* arg_type = GetExprType( node->Arg( i ) );
			if( arg_type != expected_type )
			{
				Error( node->Arg( i )->Location(), "Expected argument of type " + expected_type->ToString() + ", got expression of type " + arg_type->ToString() );
			}
		}

		BAT_RETURN( ret_type );
	}
	void SemanticAnalysis::VisitGroupExpr( GroupExpr* node )
	{
		Analyze( node->Expr() );
	}
	void SemanticAnalysis::VisitVarExpr( VarExpr* node )
	{
		Symbol* symbol = m_pSymTab->GetSymbol( node->name.lexeme );
		if( !symbol )
		{
			Error( node->name.loc, "Undefined variable '" + node->name.lexeme + "'" );
			BAT_RETURN( typeman.GetType( PrimitiveKind::Int ) );
		}

		VariableSymbol* var_symbol = symbol->AsVariable();
		if( var_symbol )
		{
			BAT_RETURN( var_symbol->VarType() );
		}

		FunctionSymbol* func_symbol = symbol->AsFunction();
		if( func_symbol )
		{
			BAT_RETURN( func_symbol->Node()->ToFuncDecl()->Signature().ReturnType() );
		}

		Error( node->name.loc, "Unhandled symbol type" );
		BAT_RETURN( typeman.GetType( PrimitiveKind::Int ) );
	}
	void SemanticAnalysis::VisitExpressionStmt( ExpressionStmt* node )
	{
		Analyze( node->Expr() );
	}
	void SemanticAnalysis::VisitBlockStmt( BlockStmt* node )
	{
		PushScope();
		for( size_t i = 0; i < node->NumStatements(); i++ )
		{
			Analyze( node->Stmt( i ) );
		}
		PopScope();
	}
	void SemanticAnalysis::VisitPrintStmt( PrintStmt* node )
	{
		Analyze( node->Expr() );
	}
	void SemanticAnalysis::VisitIfStmt( IfStmt* node )
	{
		Analyze( node->Condition() );
		Analyze( node->Then() );
		if( node->Else() ) Analyze( node->Else() );
	}
	void SemanticAnalysis::VisitWhileStmt( WhileStmt* node )
	{
		Analyze( node->Condition() );
		Analyze( node->Body() );
	}
	void SemanticAnalysis::VisitForStmt( ForStmt* node )
	{
		PushScope();
		if( node->Initializer() ) Analyze( node->Initializer() );
		if( node->Condition() )   Analyze( node->Condition() );
		if( node->Increment() )   Analyze( node->Increment() );
		Analyze( node->Body() );
		PopScope();
	}
	void SemanticAnalysis::VisitReturnStmt( ReturnStmt* node )
	{
		if( !m_pCurrentFunc )
		{
			Error( node->Location(), "Return statement used outside of function" );
			return;
		}

		Type* rettype = GetExprType( node->RetValue() );
		if( m_pCurrentFunc->Signature().ReturnType() != nullptr && rettype != m_pCurrentFunc->Signature().ReturnType() )
		{
			Error( node->Location(), "Return value type (" + rettype->ToString() + ") does not match function type (" + m_pCurrentFunc->Signature().ReturnType()->ToString() + ")" );
			return;
		}

		m_pCurrentFunc->Signature().SetReturnType( rettype );
	}
	void SemanticAnalysis::VisitVarDecl( VarDecl* node )
	{
		for( VarDecl* decl = node; decl; decl = node->Next() )
		{
			if( decl->Classifier().type != TOKEN_VAR )
			{
				Type* var_type = TokenToType( decl->Classifier() );
				if( decl->Initializer() )
				{
					Type* init_type = GetExprType( decl->Initializer() );
					if( var_type != init_type )
					{
						Error( decl->Location(), "Cannot assign expression of type " + init_type->ToString() + " to variable of type " + var_type->ToString() );
					}
					else
					{
						AddVariable( decl, decl->Identifier().lexeme, var_type );
					}
				}
				else
				{
					AddVariable( decl, decl->Identifier().lexeme, var_type );
				}
			}
			else
			{
				if( !decl->Initializer() )
				{
					Error( decl->Location(), "Could not resolve variable type" );
				}
				else
				{
					Type* init_type = GetExprType( decl->Initializer() );
					AddVariable( decl, decl->Identifier().lexeme, init_type );
				}
			}
			m_pSymTab->AddSymbol( node->Identifier().lexeme, std::make_unique<VariableSymbol>( node ) );
		}
	}
	void SemanticAnalysis::VisitFuncDecl( FuncDecl* node )
	{
		if( m_pCurrentFunc )
		{
			Error( node->Location(), "Nested functions are not permitted" );
			return;
		}

		auto& sig = node->Signature();
		AddFunction( node, sig.Identifier().lexeme );

		PushScope();
		m_pCurrentFunc = node;
		for( size_t i = 0; i < sig.NumParams(); i++ )
		{
			if( !sig.ParamDefault( i ) && sig.ParamType( i ).type == TOKEN_VAR )
			{
				Error( sig.ParamType( i ).loc, "Could not resolve variable type" );
			}
			else if( sig.ParamDefault( i ) &&
				sig.ParamType( i ).type != TOKEN_VAR )
			{
				Type* param_type = TokenToType( sig.ParamType( i ) );
				Type* default_expr_type = GetExprType( sig.ParamDefault( i ) );
				if( param_type != default_expr_type )
				{
					Error( sig.ParamType( i ).loc, std::string( "Cannot assign expression of type " ) + default_expr_type->ToString() +
						" to parameter of type " + param_type->ToString() );
				}
				else
				{
					AddVariable( node, sig.ParamIdent( i ).lexeme, param_type );
				}
			}
			else if( sig.ParamDefault( i ) )
			{
				Type* default_expr_type = GetExprType( sig.ParamDefault( i ) );
				AddVariable( node, sig.ParamIdent( i ).lexeme, default_expr_type );
			}
			else
			{
				Type* param_type = TokenToType( sig.ParamType( i ) );
				AddVariable( node, sig.ParamIdent( i ).lexeme, param_type );
			}
		}

		if( sig.ReturnIdentifier().type != TOKEN_DEF )
		{
			sig.SetReturnType( TokenToType( sig.ReturnIdentifier() ) );
		}

		Analyze( node->Body() );
		m_pCurrentFunc = nullptr;
		PopScope();
	}
}
