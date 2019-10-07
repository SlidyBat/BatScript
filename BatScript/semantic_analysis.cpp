#include "semantic_analysis.h"

#include <fstream>
#include "type_manager.h"
#include "errorsys.h"
#include "lexer.h"
#include "parser.h"
#include "memory_stream.h"

namespace Bat
{
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
		return e->Type();
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
		m_pSymTab->AddSymbol( name, std::make_unique<FunctionSymbol>( node, FunctionKind::Script ) );
	}
	void SemanticAnalysis::AddNative( NativeStmt* node, const std::string& name )
	{
		if( Symbol* s = m_pSymTab->GetSymbol( name ) )
		{
			if( FunctionSymbol* native = s->ToFunction() )
			{
				// Function with this name already exists // TODO: Consider allowing overloading by comparing signature parameters too
				ErrorSys::Report( node->Location().Line(), node->Location().Column(), "'" + node->Signature().Identifier().lexeme + "' already defined" );
			}
			else
			{
				// Symbol already exists, but not as a function. Report that.
				ErrorSys::Report( node->Location().Line(), node->Location().Column(), "'" + node->Signature().Identifier().lexeme + "' already defined as a variable" );
			}
		}
		else
		{
			// Symbol does not exist yet and is safe to add
			m_pSymTab->AddSymbol( name, std::make_unique<FunctionSymbol>( node, FunctionKind::Native ) );
		}
	}
	Type* SemanticAnalysis::Coerce( Type* from, Type* to )
	{
		// Handle exact matches
		if( from == to )
		{
			return from;
		}

		if( from->IsPrimitive() && to->IsPrimitive() &&
			from->AsPrimitive()->PrimKind() == to->AsPrimitive()->PrimKind() )
		{
			return from;
		}

		if( from->IsArray() && to->IsArray() )
		{
			ArrayType* from_arr = from->AsArray();
			ArrayType* to_arr = to->AsArray();

			if( to_arr->HasFixedSize() && to_arr->FixedSize() != from_arr->FixedSize() )
			{
				return nullptr;
			}

			return from;
		}

		// Handle cases that need implicit casts
		PrimitiveType* pfrom = from->ToPrimitive();
		PrimitiveType* pto = to->ToPrimitive();
		if( pto && pfrom )
		{
			if( pfrom->PrimKind() == PrimitiveKind::Int && pto->PrimKind() == PrimitiveKind::Float )
			{
				return to;
			}
		}

		return nullptr;
	}
	void SemanticAnalysis::VisitIntLiteral( IntLiteral* node )
	{
		node->SetType( typeman.NewPrimitive( PrimitiveKind::Int ) );
	}
	void SemanticAnalysis::VisitFloatLiteral( FloatLiteral* node )
	{
		node->SetType( typeman.NewPrimitive( PrimitiveKind::Float ) );
	}
	void SemanticAnalysis::VisitStringLiteral( StringLiteral* node )
	{
		node->SetType( typeman.NewPrimitive( PrimitiveKind::String ) );
	}
	void SemanticAnalysis::VisitTokenLiteral( TokenLiteral* node )
	{
		switch( node->value )
		{
			case TOKEN_TRUE:
			case TOKEN_FALSE:
				node->SetType( typeman.NewPrimitive( PrimitiveKind::Bool ) );
				return;
		}
		assert( false );
		node->SetType( typeman.NewPrimitive( PrimitiveKind::Int ) ); // TODO: ???
	}
	void SemanticAnalysis::VisitArrayLiteral( ArrayLiteral* node )
	{
		Type* inner = nullptr;
		for( size_t i = 0; i < node->NumValues(); i++ )
		{
			Type* curr_type = GetExprType( node->ValueAt( i ) );
			if( !inner )
			{
				inner = curr_type;
			}

			Type* coerced = Coerce( curr_type, inner );
			if( !coerced )
			{
				Error( node->ValueAt( i )->Location(), "Expression of type '" + curr_type->ToString() + "' does not match previous type of '" + inner->ToString() + "' in array literal" );
			}
			else
			{
				// Implicit casting not supported for array types, either they're the same type or they aren't
				assert( coerced == curr_type );
			}
		}

		// They evaluate to an array that is unsized so statements like:
		//   var x = [5, 2, 1]
		// Will infer to x being a dynamic length array
		node->SetType( typeman.NewArray( inner, ArrayType::UNSIZED ) );
	}
	Type* SemanticAnalysis::PrimitiveBinary( PrimitiveType* left, PrimitiveType* right, TokenType op )
	{
		if( left->PrimKind() == right->PrimKind() )
		{
			return left;
		}

		if( left->PrimKind() == PrimitiveKind::Float )
		{
			return left;
		}

		switch( op )
		{
		case TOKEN_EQUAL:
		case TOKEN_PLUS_EQUAL:
		case TOKEN_MINUS_EQUAL:
		case TOKEN_ASTERISK_EQUAL:
		case TOKEN_SLASH_EQUAL:
		case TOKEN_PERCENT_EQUAL:
		case TOKEN_AMP_EQUAL:
		case TOKEN_HAT_EQUAL:
		case TOKEN_BAR_EQUAL:
			// No implicit casting from float to int for assignments
			return nullptr;
		}

		// Other operations are ok to implicitly cast
		if( right->PrimKind() == PrimitiveKind::Float )
		{
			return right;
		}

		assert( false );
		return nullptr;
	}
	Type* SemanticAnalysis::ArrayBinary( ArrayType* left, Type* right, TokenType op )
	{
		switch( op )
		{
		case TOKEN_MINUS_EQUAL:
		case TOKEN_ASTERISK_EQUAL:
		case TOKEN_SLASH_EQUAL:
		case TOKEN_PERCENT_EQUAL:
		case TOKEN_AMP_EQUAL:
		case TOKEN_HAT_EQUAL:
		case TOKEN_BAR_EQUAL:
			return nullptr;
		}

		if( ArrayType* right_arr = right->ToArray() )
		{
			// Can only do direct assignment between arrays
			if( op != TOKEN_EQUAL )
			{
				return nullptr;
			}

			// Can't assign array to a fixed size array, unless its also a fixed array with the same size
			if( left->HasFixedSize() && left->FixedSize() != right_arr->FixedSize() )
			{
				return nullptr;
			}

			return left;
		}
		else if( left->HasFixedSize() )
		{
			// No assignment operations allowed on fixed size arrays (unless right hand side is also array with same size)
			return nullptr;
		}

		return nullptr;
	}
	void SemanticAnalysis::VisitBinaryExpr( BinaryExpr* node )
	{
		switch( node->Op() )
		{
		case TOKEN_EQUAL:
		case TOKEN_PLUS_EQUAL:
		case TOKEN_MINUS_EQUAL:
		case TOKEN_ASTERISK_EQUAL:
		case TOKEN_SLASH_EQUAL:
		case TOKEN_PERCENT_EQUAL:
		case TOKEN_AMP_EQUAL:
		case TOKEN_HAT_EQUAL:
		case TOKEN_BAR_EQUAL:
			if( !node->Left()->IsLValue() )
			{
				Error( node->Left()->Location(), "Expression is not modifiable lvalue" );
			}

			break;
		}

		Type* left = GetExprType( node->Left() );
		Type* right = GetExprType( node->Right() );

		if( IsNumericType( left ) && IsNumericType( right ) )
		{
			Type* result = PrimitiveBinary( left->AsPrimitive(), right->AsPrimitive(), node->Op() );
			if( result )
			{
				if( !IsSameType( result, left ) )
				{
					auto cast = std::make_unique<CastExpr>( node->TakeLeft(), result );
					node->SetLeft( std::move( cast ) );
				}
				if( !IsSameType( result, right ) )
				{
					auto cast = std::make_unique<CastExpr>( node->TakeRight(), result );
					node->SetRight( std::move( cast ) );
				}

				node->SetType( result );
				return;
			}
		}
		else if( left->IsArray() )
		{
			Type* result = ArrayBinary( left->AsArray(), right, node->Op() );
			if( result )
			{
				node->SetType( result );
				return;
			}
		}

		Error( node->Location(), std::string( "Cannot use operator '" ) + TokenTypeToString( node->Op() ) + "' on expressions of types " + left->ToString()
			+ " and " + right->ToString() );

		node->SetType( left );
	}
	void SemanticAnalysis::VisitUnaryExpr( UnaryExpr* node )
	{
		Type* right = GetExprType( node->Right() );
		if( !IsNumericType( right ) )
		{
			Error( node->Location(), std::string( "Cannot use operator '" ) + TokenTypeToString( node->Op() ) + "' on expression of type " + right->ToString() );
		}

		node->SetType( right );
	}
	void SemanticAnalysis::VisitCallExpr( CallExpr* node )
	{
		Type* t = GetExprType( node->Function() );
		// TODO: Find better way to check if expression is callable, not all will be variable expressions
		VarExpr* callee = node->Function()->ToVarExpr();
		if( !callee )
		{
			Error( node->Location(),  "Expression is not callable" );
			node->SetType( t );
			return;
		}
		Symbol* symbol = m_pSymTab->GetSymbol( callee->Identifier().lexeme );
		if( !symbol || !symbol->IsFunction() )
		{
			Error( node->Location(), callee->Identifier().lexeme + " is not a function" );
			node->SetType( t );
			return;
		}

		FunctionSymbol* func_symbol = symbol->ToFunction();

		auto& sig = func_symbol->Signature();
		Type* ret_type = sig.ReturnType();

		if( !sig.VarArgs() && (node->NumArgs() != sig.NumParams()) ) // TODO: handle defaults
		{
			Error( node->Location(), "Expected " + std::to_string( sig.NumParams() ) + " argument(s), got " + std::to_string( node->NumArgs() ) );
		}
		else if( sig.VarArgs() && (node->NumArgs() < sig.NumParams()) ) // It's fine to have more params if its a varargs func
		{
			Error( node->Location(), "Expected at least " + std::to_string( sig.NumParams() ) + " argument(s), got " + std::to_string( node->NumArgs() ) );
		}
		else
		{
			for( size_t i = 0; i < sig.NumParams(); i++ )
			{
				Type* expected_type = TypeSpecifierToType( sig.ParamType( i ) );
				Type* arg_type = GetExprType( node->Arg( i ) );

				Type* coerced = Coerce( arg_type, expected_type );
				if( !coerced )
				{
					Error( node->Arg( i )->Location(), "Expected argument of type " + expected_type->ToString() + ", got expression of type " + arg_type->ToString() );
				}

				// Check if a cast is needed
				else if( coerced != arg_type )
				{
					auto cast = std::make_unique<CastExpr>( node->TakeArg( i ), expected_type );
					node->SetArg( i, std::move( cast ) );
				}
			}
		}

		node->SetType( ret_type );
	}
	void SemanticAnalysis::VisitIndexExpr( IndexExpr* node )
	{
		Type* arr_type = GetExprType( node->Array() );
		if( !arr_type->IsArray() )
		{
			Error( node->Array()->Location(), "Expression does not evaluate to array" );
			node->SetType( arr_type );
			return;
		}

		Type* inner = arr_type->AsArray()->Inner();

		Type* int_type = typeman.NewPrimitive( PrimitiveKind::Int );
		Type* index_type = GetExprType( node->Index() );
		Type* coerced = Coerce( index_type, int_type );
		if( !coerced )
		{
			Error( node->Index()->Location(), "Array index must evaluate to integer" );
		}
		else if( coerced != index_type )
		{
			auto cast = std::make_unique<CastExpr>( node->TakeIndex(), int_type );
			node->SetIndex( std::move( cast ) );
		}

		node->SetType( inner );
	}
	void SemanticAnalysis::VisitCastExpr( CastExpr* node )
	{
		// Not yet implemented
		assert( false );
	}
	void SemanticAnalysis::VisitGroupExpr( GroupExpr* node )
	{
		node->SetType( GetExprType( node->Expr() ) );
	}
	void SemanticAnalysis::VisitVarExpr( VarExpr* node )
	{
		Symbol* symbol = m_pSymTab->GetSymbol( node->Identifier().lexeme );
		if( !symbol )
		{
			Error( node->Identifier().loc, "Undefined variable '" + node->Identifier().lexeme + "'" );
			node->SetType( typeman.NewPrimitive( PrimitiveKind::Int ) );
			return;
		}

		VariableSymbol* var_symbol = symbol->AsVariable();
		if( var_symbol )
		{
			node->SetType( var_symbol->VarType() );
			return;
		}

		FunctionSymbol* func_symbol = symbol->AsFunction();
		if( func_symbol )
		{
			node->SetType( func_symbol->Signature().ReturnType() );
			return;
		}

		Error( node->Identifier().loc, "Unhandled symbol type" );
		node->SetType( typeman.NewPrimitive( PrimitiveKind::Int ) );
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

		Type* rettype = node->RetExpr() ? GetExprType( node->RetExpr() ) : typeman.NewPrimitive( PrimitiveKind::Void );
		if( m_pCurrentFunc->Signature().ReturnType() != nullptr )
		{
			Type* coerced = Coerce( rettype, m_pCurrentFunc->Signature().ReturnType() );

			if( !coerced )
			{
				Error( node->Location(), "Return value type (" + rettype->ToString() + ") does not match function type (" + m_pCurrentFunc->Signature().ReturnType()->ToString() + ")" );
			}
			else if( coerced != rettype )
			{
				auto cast = std::make_unique<CastExpr>( node->TakeRetExpr(), m_pCurrentFunc->Signature().ReturnType() );
				node->SetRetExpr( std::move( cast ) );
			}
		}
		else
		{
			m_pCurrentFunc->Signature().SetReturnType( rettype );
		}
	}
	void SemanticAnalysis::VisitImportStmt( ImportStmt* node )
	{
		std::string filename = node->ModuleName() + ".bat";
		if( !std::ifstream( filename ) )
		{
			filename = node->ModuleName() + ".bs";
			if( !std::ifstream( filename ) )
			{
				ErrorSys::Report( node->Location().Line(), node->Location().Column(), "Module '" + node->ModuleName() + "' not found" );
			}
		}

		auto source = MemoryStream::FromFile( filename, FileMode::TEXT );

		Lexer l( source.Base() );
		auto tokens = l.Scan();

		if( ErrorSys::HadError() ) return;

		Parser p( std::move( tokens ) );
		std::vector<std::unique_ptr<Statement>> res = p.Parse();

		if( ErrorSys::HadError() ) return;

		for( size_t i = 0; i < res.size(); i++ )
		{
			Analyze( res[i].get() );
			m_pStatements.push_back( std::move( res[i] ) );
		}
	}
	void SemanticAnalysis::VisitNativeStmt( NativeStmt* node )
	{
		if( m_pCurrentFunc )
		{
			Error( node->Location(), "Natives must be declared in global scope" );
			return;
		}

		auto& sig = node->Signature();
		AddNative( node, sig.Identifier().lexeme );
		sig.SetReturnType( TypeSpecifierToType( sig.ReturnTypeSpec() ) );
	}
	void SemanticAnalysis::VisitVarDecl( VarDecl* node )
	{
		if( node->TypeSpec().TypeName().type != TOKEN_VAR )
		{
			Type* var_type = TypeSpecifierToType( node->TypeSpec() );

			if( var_type->IsPrimitive() && var_type->ToPrimitive()->PrimKind() == PrimitiveKind::Void )
			{
				Error( node->Location(), "'" + var_type->ToString() + "' is an invalid variable type" );
				node->SetType( var_type );
				return;
			}

			if( node->Initializer() )
			{
				Type* init_type = GetExprType( node->Initializer() );
				Type* coerced = Coerce( init_type, var_type );
				if( !coerced )
				{
					Error( node->Location(), "Cannot assign expression of type " + init_type->ToString() + " to variable of type " + var_type->ToString() );
				}
				else if( coerced != init_type )
				{
					auto cast = std::make_unique<CastExpr>( node->TakeInitializer(), var_type );
					node->SetInitializer( std::move( cast ) );
				}

				AddVariable( node, node->Identifier().lexeme, var_type );
			}
			else
			{
				AddVariable( node, node->Identifier().lexeme, var_type );
			}

			node->SetType( var_type );
		}
		else
		{
			if( !node->Initializer() )
			{
				Error( node->Location(), "Could not resolve variable type" );
			}
			else
			{
				Type* init_type = GetExprType( node->Initializer() );
				AddVariable( node, node->Identifier().lexeme, init_type );

				node->SetType( init_type );
			}
		}
	}
	void SemanticAnalysis::VisitFuncDecl( FuncDecl* node )
	{
		if( !InGlobalScope() )
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
			if( !sig.ParamDefault( i ) && sig.ParamType( i ).TypeName().type == TOKEN_VAR )
			{
				Error( sig.ParamType( i ).TypeName().loc, "Could not resolve variable type" );
			}
			else if( sig.ParamDefault( i ) && sig.ParamType( i ).TypeName().type != TOKEN_VAR )
			{
				Type* param_type = TypeSpecifierToType( sig.ParamType( i ) );
				Type* default_expr_type = GetExprType( sig.ParamDefault( i ) );
				Type* coerced = Coerce( default_expr_type, param_type );
				if( !coerced )
				{
					Error( sig.ParamType( i ).TypeName().loc, std::string( "Cannot assign expression of type " ) + default_expr_type->ToString() +
						" to parameter of type " + param_type->ToString() );
				}
				else if( coerced != default_expr_type )
				{
					auto cast = std::make_unique<CastExpr>( sig.TakeParamDefault( i ), param_type );
					sig.SetParamDefault( i, std::move( cast ) );
				}

				// Add the variable regardless of coercion success so that the errors don't pile up
				AddVariable( node, sig.ParamIdent( i ).lexeme, param_type );
			}
			else if( sig.ParamDefault( i ) )
			{
				Type* default_expr_type = GetExprType( sig.ParamDefault( i ) );
				AddVariable( node, sig.ParamIdent( i ).lexeme, default_expr_type );
			}
			else
			{
				Type* param_type = TypeSpecifierToType( sig.ParamType( i ) );
				AddVariable( node, sig.ParamIdent( i ).lexeme, param_type );
			}
		}

		if( sig.ReturnTypeSpec().TypeName().type != TOKEN_DEF )
		{
			sig.SetReturnType( TypeSpecifierToType( sig.ReturnTypeSpec() ) );
		}

		Analyze( node->Body() );

		// Couldn't deduce return type from returns because there weren't any
		// No returns = void function
		if( sig.ReturnType() == nullptr )
		{
			sig.SetReturnType( typeman.NewPrimitive( PrimitiveKind::Void ) );
		}

		m_pCurrentFunc = nullptr;
		PopScope();
	}
}
