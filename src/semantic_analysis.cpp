#include "semantic_analysis.h"

#include <fstream>
#include "type_manager.h"
#include "errorsys.h"
#include "lexer.h"
#include "parser.h"
#include "memory_stream.h"

namespace Bat
{
	static bool IsNumericType( Type* type );
	static bool IsStringType( Type* type );
	static bool IsIntegralType( Type* type );
	static bool IsFloatType( Type* type );

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
	void SemanticAnalysis::AddVariable( AstNode* node, const Token& name, Type* type )
	{
		if( type->IsPrimitive() && type->ToPrimitive()->PrimKind() == PrimitiveKind::Void )
		{
			Error( name.loc, "'" + type->ToString() + "' is an invalid variable type" );
		}
		m_pSymTab->AddSymbol( name.lexeme, std::make_unique<VariableSymbol>( node, type ) );
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
				Error( node->Location(), "'" + node->Signature().Identifier().lexeme + "' already defined" );
			}
			else
			{
				// Symbol already exists, but not as a function. Report that.
				Error( node->Location(), "'" + node->Signature().Identifier().lexeme + "' already defined as a variable" );
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
			//ArrayType* from_arr = from->AsArray();
			//ArrayType* to_arr = to->AsArray();

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
			
			default:
				assert( false );
				node->SetType( typeman.NewPrimitive( PrimitiveKind::Int ) ); // TODO: ???
		}
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

		node->SetType( typeman.NewArray( inner ) );
	}
	Type* SemanticAnalysis::PrimitiveBinary( PrimitiveType* left, PrimitiveType* right, TokenType op, Type** coerce_to )
	{
		*coerce_to = nullptr;

		switch( op )
		{
		// Arithmetic operators
		case TOKEN_PLUS:
		case TOKEN_MINUS:
		case TOKEN_ASTERISK:
		case TOKEN_SLASH:
			if( !IsNumericType( left ) || !IsNumericType( right ) ) return nullptr;

			if( IsFloatType( left ) )
			{
				*coerce_to = left;
				return left;
			}
			if( IsFloatType( right ) )
			{
				*coerce_to = right;
				return right;
			}
			if( IsIntegralType( left ) )
			{
				*coerce_to = left;
				return left;
			}
			if( IsIntegralType( right ) )
			{
				*coerce_to = right;
				return right;
			}

		// Bitwise operators (+ mod since its also integral type only)
		case TOKEN_BAR:
		case TOKEN_HAT:
		case TOKEN_AMP:
		case TOKEN_LESS_LESS:
		case TOKEN_GREATER_GREATER:
		case TOKEN_PERCENT:
			if( !IsIntegralType( left ) || !IsIntegralType( right ) ) return nullptr;

			// TODO: Make this return value with larger size when/if more integer types are added
			//       For now they're both just `int` and doesn't matter which we return
			*coerce_to = left;
			return left;


		// Comparison operators
		case TOKEN_EQUAL_EQUAL:
		case TOKEN_EXCLMARK_EQUAL:
		case TOKEN_LESS:
		case TOKEN_LESS_EQUAL:
		case TOKEN_GREATER:
		case TOKEN_GREATER_EQUAL:
			if( !IsNumericType( left ) || !IsNumericType( right ) ) return nullptr;

			if( IsFloatType( left ) )
			{
				*coerce_to = left;
			}
			else if( IsFloatType( right ) )
			{
				*coerce_to = right;
			}
			else if( IsIntegralType( left ) )
			{
				*coerce_to = left;
			}
			else if( IsIntegralType( right ) )
			{
				*coerce_to = right;
			}

			return typeman.NewPrimitive( PrimitiveKind::Bool );

		// Assignment operators
		case TOKEN_EQUAL:
		case TOKEN_PLUS_EQUAL:
		case TOKEN_MINUS_EQUAL:
		case TOKEN_ASTERISK_EQUAL:
		case TOKEN_SLASH_EQUAL:
		case TOKEN_PERCENT_EQUAL:
		case TOKEN_AMP_EQUAL:
		case TOKEN_HAT_EQUAL:
		case TOKEN_BAR_EQUAL:
			if( !IsNumericType( left ) || !IsNumericType( right ) ) return nullptr;

			// No implicit casting from float to int for assignments
			if( IsFloatType( left ) )
			{
				*coerce_to = left;
				return left;
			}
			else if( IsIntegralType( left ) )
			{
				*coerce_to = right;
				return left;
			}

			return nullptr;
		
		default:
			assert( false );
			return nullptr;
		}
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
		
		default:
			break;
		}

		if( ArrayType* right_arr = right->ToArray() )
		{
			// Can only do direct assignment between arrays
			if( op != TOKEN_EQUAL )
			{
				return nullptr;
			}

			return left;
		}

		return nullptr;
	}
	void SemanticAnalysis::VisitBinaryExpr( BinaryExpr* node )
	{
		Type* left = GetExprType( node->Left() );
		Type* right = GetExprType( node->Right() );

		if( IsNumericType( left ) && IsNumericType( right ) )
		{
			Type* coerce_to;
			Type* result = PrimitiveBinary( left->AsPrimitive(), right->AsPrimitive(), node->Op(), &coerce_to );
			if( result )
			{
				if( !IsSameType( coerce_to, left ) )
				{
					auto cast = std::make_unique<CastExpr>( node->TakeLeft(), coerce_to );
					node->SetLeft( std::move( cast ) );
				}
				if( !IsSameType( coerce_to, right ) )
				{
					auto cast = std::make_unique<CastExpr>( node->TakeRight(), coerce_to );
					node->SetRight( std::move( cast ) );
				}

				node->SetType( result );
				return;
			}
		}
		else if( IsStringType( left ) && IsStringType( right ) )
		{
			node->SetType( left );
			return;
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

		Error( node->Left()->Location(), std::string( "Cannot use operator '" ) + TokenTypeToString( node->Op() ) + "' on expressions of types " + left->ToString()
			+ " and " + right->ToString() );

		node->SetType( left );
	}

	static bool IsNumericType( Type* type )
	{
		if( !type ) return false;

		switch( type->Kind() )
		{
		case TypeKind::Array:
		case TypeKind::Function:
		case TypeKind::Class:
			return false;
		case TypeKind::Primitive:
			switch( type->AsPrimitive()->PrimKind() )
			{
			case PrimitiveKind::Void:
			case PrimitiveKind::String:
				return false;
			case PrimitiveKind::Bool: // (?)
			case PrimitiveKind::Int:
			case PrimitiveKind::Float:
				return true;
			}
		}

		assert( false );
		return false;
	}

	static bool IsStringType( Type* type )
	{
		if( !type ) return false;

		switch( type->Kind() )
		{
		case TypeKind::Primitive:
			switch( type->AsPrimitive()->PrimKind() )
			{
			case PrimitiveKind::String:
				return true;
			default:
				return false;
			}
		default:
			return false;
		}
	}

	static bool IsIntegralType( Type* type )
	{
		if( !type ) return false;

		switch( type->Kind() )
		{
		case TypeKind::Array:
		case TypeKind::Function:
		case TypeKind::Class:
			return false;
		case TypeKind::Primitive:
			switch( type->AsPrimitive()->PrimKind() )
			{
			case PrimitiveKind::Void:
			case PrimitiveKind::Bool:
			case PrimitiveKind::String:
			case PrimitiveKind::Float:
				return false;
			case PrimitiveKind::Int:
				return true;
			}
		}

		assert( false );
		return false;
	}

	static bool IsFloatType( Type* type )
	{
		if( !type ) return false;

		switch( type->Kind() )
		{
		case TypeKind::Array:
		case TypeKind::Function:
		case TypeKind::Class:
			return false;
		case TypeKind::Primitive:
			switch( type->AsPrimitive()->PrimKind() )
			{
			case PrimitiveKind::Void:
			case PrimitiveKind::Bool:
			case PrimitiveKind::String:
			case PrimitiveKind::Int:
				return false;
			case PrimitiveKind::Float:
				return true;
			}
		}

		assert( false );
		return false;
	}

	void SemanticAnalysis::VisitUnaryExpr( UnaryExpr* node )
	{
		Type* right = GetExprType( node->Right() );

		switch( node->Op() )
		{
		case TOKEN_MINUS:
			if( !IsNumericType( right ) )
			{
				Error( node->Location(), std::string( "Cannot use operator '" ) + TokenTypeToString( node->Op() ) + "' on expression of type " + right->ToString() );
			}
			node->SetType( right );
			break;
		case TOKEN_EXCLMARK:
			if( !IsNumericType( right ) )
			{
				Error( node->Location(), std::string( "Cannot use operator '" ) + TokenTypeToString( node->Op() ) + "' on expression of type " + right->ToString() );
			}
			node->SetType( typeman.NewPrimitive( PrimitiveKind::Bool ) );
			break;
		case TOKEN_TILDE:
			if( !right->IsPrimitive() || right->AsPrimitive()->PrimKind() != PrimitiveKind::Int )
			{
				Error( node->Location(), std::string( "Cannot use operator '" ) + TokenTypeToString( node->Op() ) + "' on expression of type " + right->ToString() );
			}
			node->SetType( right );
			break;
		
		default:
			assert( !"Unhandled unary operator" );
			break;
		}
	}
	void SemanticAnalysis::VisitCallExpr( CallExpr* node )
	{
		Type* t = GetExprType( node->Function() );
		bool is_method_call = false;
		Type* base_type = nullptr;
		FunctionSignature* sig = nullptr;

		if( FunctionType* func_type = t->ToFunction() )
		{
			sig = func_type->Signature();
		}
		else if( ClassType* class_type = t->ToClass() )
		{
			Symbol* constructor = class_type->SymTab()->GetSymbolShallow( "constructor" );
			if( constructor && constructor->IsFunction() )
			{
				is_method_call = true;
				base_type = class_type;

				if( FuncDecl* constructor_decl = constructor->Node()->ToFuncDecl() )
					sig = &constructor_decl->Signature();
				else if( NativeStmt* constructor_native = constructor->Node()->ToNativeStmt() )
					sig = &constructor_native->Signature();
				else
					Error( node->Location(), "Unhandled constructor type" );
			}
		}
		else
		{
			Error( node->Location(), "Expression is not callable" );
			node->SetType( t );
			return;
		}


		if( AttrExpr* attr = node->Function()->ToAttrExpr() )
		{
			if( VarExpr* var = attr->Base()->ToVarExpr() )
			{
				Symbol* sym = m_pSymTab->GetSymbol( var->Identifier().lexeme );
				if( sym && sym->IsVariable() )
				{
					is_method_call = true;
					base_type = GetExprType( attr->Base() );
				}
			}
		}

		// Having nullptr sig can only mean we are calling an implicit constructor of a class
		bool var_args = sig ? sig->VarArgs() : false;
		size_t num_params = sig ? sig->NumParams() : 0;


		if( !var_args && (node->NumArgs() + is_method_call != num_params) ) // TODO: handle defaults
		{
			Error( node->Location(), "Expected " + std::to_string( num_params ) + " argument(s), got " + std::to_string( node->NumArgs() ) );
		}
		else if( var_args && (node->NumArgs() + is_method_call < num_params) ) // It's fine to have more params if its a varargs func
		{
			Error( node->Location(), "Expected at least " + std::to_string( num_params ) + " argument(s), got " + std::to_string( node->NumArgs() ) );
		}
		else
		{
			for( size_t i = 0; i < num_params; i++ )
			{
				Type* expected_type = TypeSpecifierToType( sig->ParamType( i ) );

				Type* arg_type;
				if( is_method_call )
				{
					arg_type = (i == 0) ? base_type : GetExprType( node->Arg( i - 1 ) );
				}
				else
				{
					arg_type = GetExprType( node->Arg( i ) );
				}
				
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

		if( !t->IsClass() )
			node->SetType( sig->ReturnType() );
		else
			node->SetType( t );
	}
	void SemanticAnalysis::VisitIndexExpr( IndexExpr* node )
	{
		Type* node_type = GetExprType( node->Array() );
		if( !node_type->IsArray() &&
			!( node_type->IsPrimitive() && node_type->AsPrimitive()->PrimKind() == PrimitiveKind::String ) )
		{
			Error( node->Array()->Location(), "Expression does not evaluate to array or string" );
			node->SetType( node_type );
			return;
		}

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

		if ( node_type->IsArray() )
		{
			Type* inner = node_type->AsArray()->Inner();
			node->SetType( inner );
		}
		else
		{
			node->SetType( node_type );
		}
	}
	void SemanticAnalysis::VisitSliceExpr( SliceExpr* node )
	{
		Type* node_type = GetExprType( node->Array() );
		if( !node_type->IsArray() &&
			!( node_type->IsPrimitive() && node_type->AsPrimitive()->PrimKind() == PrimitiveKind::String ) )
		{
			Error( node->Array()->Location(), "Expression does not evaluate to array or string" );
			node->SetType( node_type );
			return;
		}

		Type* int_type = typeman.NewPrimitive( PrimitiveKind::Int );
		if ( node->BeginIndex() )
		{
			Type* index_type = GetExprType( node->BeginIndex() );
			Type* coerced = Coerce( index_type, int_type );
			if( !coerced )
			{
				Error( node->BeginIndex()->Location(), "Slice begin index must evaluate to integer" );
			}
			else if( coerced != index_type )
			{
				auto cast = std::make_unique<CastExpr>( node->TakeBeginIndex(), int_type );
				node->SetBeginIndex( std::move( cast ) );
			}
		}
		if ( node->EndIndex() )
		{
			Type* index_type = GetExprType( node->EndIndex() );
			Type* coerced = Coerce( index_type, int_type );
			if( !coerced )
			{
				Error( node->EndIndex()->Location(), "Slice end index must evaluate to integer" );
			}
			else if( coerced != index_type )
			{
				auto cast = std::make_unique<CastExpr>( node->TakeEndIndex(), int_type );
				node->SetEndIndex( std::move( cast ) );
			}
		}

		node->SetType( node_type );
	}
	void SemanticAnalysis::VisitAttrExpr( AttrExpr* node )
	{
		if( VarExpr* var = node->Base()->ToVarExpr() )
		{
			Symbol* sym = m_pSymTab->GetSymbol( var->Identifier().lexeme );
			if( !sym )
			{
				Error( node->Attr().loc, "Undefined variable '"+var->Identifier().lexeme+"'" );
				return;
			}

			const SymbolTable* symtab = nullptr;

			if( ModuleSymbol* module = sym->ToModule() )
			{
				symtab = module->SymTab();
			}
			else if( VariableSymbol* var_sym = sym->ToVariable() )
			{
				Type* var_type = var_sym->VarType();
				if( !var_type )
				{
					Error( node->Attr().loc, "Unknown variable type" );
					node->SetType( typeman.NewPrimitive( PrimitiveKind::Int ) );
					return;
				}

				ClassType* class_type = var_type->ToClass();
				if( !class_type )
				{
					Error( node->Attr().loc, "No such attribute" );
					node->SetType( typeman.NewPrimitive( PrimitiveKind::Int ) );
					return;
				}

				symtab = class_type->SymTab();
			}
			else if( ClassSymbol* class_sym = sym->ToClass() )
			{
				symtab = class_sym->SymTab();
			}

			if( !symtab )
			{
				Error( node->Attr().loc, "No such attribute" );
				node->SetType( typeman.NewPrimitive( PrimitiveKind::Int ) );
				return;
			}

			Symbol* symbol = symtab->GetSymbol( node->Attr().lexeme );
			if( !symbol )
			{
				Error( node->Attr().loc, "Undefined variable '" + node->Attr().lexeme + "'" );
				node->SetType( typeman.NewPrimitive( PrimitiveKind::Int ) );
				return;
			}

			if( VariableSymbol* var_symbol = symbol->ToVariable() )
			{
				if( sym->IsClass() )
				{
					Error( node->Attr().loc, "Non-static field is inaccessible" );
					node->SetType( typeman.NewPrimitive( PrimitiveKind::Int ) );
					return;
				}
				
				node->SetType( var_symbol->VarType() );
				return;
			}

			if( FunctionSymbol* func_symbol = symbol->ToFunction() )
			{
				node->SetType( typeman.NewFunction( &func_symbol->Signature() ) );
				return;
			}

			if( ClassSymbol* class_symbol = symbol->ToClass() )
			{
				node->SetType( typeman.GetType( class_symbol->Node()->AsClassDecl()->Name().lexeme ) );
				return;
			}

			if( ModuleSymbol* mod_symbol = symbol->ToModule() )
			{
				Error( node->Attr().loc, "Cannot use module as variable" );
				node->SetType( typeman.NewPrimitive( PrimitiveKind::Int ) );
				return;
			}

			Error( node->Attr().loc, "Unhandled symbol type" );
			node->SetType( typeman.NewPrimitive( PrimitiveKind::Int ) );
		}
		else
		{
			Error( node->Attr().loc, "No such attribute" );
		}
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

		if( VariableSymbol* var_symbol = symbol->ToVariable() )
		{
			node->SetType( var_symbol->VarType() );
			return;
		}

		if( FunctionSymbol* func_symbol = symbol->ToFunction() )
		{
			node->SetType( typeman.NewFunction( &func_symbol->Signature() ) );
			return;
		}

		if( ClassSymbol* class_symbol = symbol->ToClass() )
		{
			node->SetType( typeman.GetType( class_symbol->Node()->AsClassDecl()->Name().lexeme ) );
			return;
		}

		if( ModuleSymbol* mod_symbol = symbol->ToModule() )
		{
			Error( node->Identifier().loc, "Cannot use module as variable" );
			node->SetType( typeman.NewPrimitive( PrimitiveKind::Int ) );
			return;
		}

		Error( node->Identifier().loc, "Unhandled symbol type" );
		node->SetType( typeman.NewPrimitive( PrimitiveKind::Int ) );
	}
	void SemanticAnalysis::VisitExpressionStmt( ExpressionStmt* node )
	{
		Analyze( node->Expr() );
	}
	void SemanticAnalysis::VisitAssignStmt( AssignStmt* node )
	{
		if( !node->Left()->IsLValue() )
		{
			Error( node->Left()->Location(), "Expression is not modifiable lvalue" );
		}

		Type* left = GetExprType( node->Left() );
		Type* right = GetExprType( node->Right() );

		if( IsNumericType( left ) && IsNumericType( right ) )
		{
			Type* coerce_to;
			Type* result = PrimitiveBinary( left->AsPrimitive(), right->AsPrimitive(), node->Op(), &coerce_to );
			if( result )
			{
				if( !IsSameType( coerce_to, right ) )
				{
					auto cast = std::make_unique<CastExpr>( node->TakeRight(), coerce_to );
					node->SetRight( std::move( cast ) );
				}
			}
		}
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
		Statement* old_loop = m_pCurrentLoop;
		m_pCurrentLoop = node;

		Analyze( node->Condition() );
		Analyze( node->Body() );

		m_pCurrentLoop = old_loop;
	}
	void SemanticAnalysis::VisitForStmt( ForStmt* node )
	{
		Statement* old_loop = m_pCurrentLoop;
		m_pCurrentLoop = node;

		PushScope();
		if( node->Initializer() ) Analyze( node->Initializer() );
		if( node->Condition() )   Analyze( node->Condition() );
		if( node->Increment() )   Analyze( node->Increment() );
		Analyze( node->Body() );
		PopScope();

		m_pCurrentLoop = old_loop;
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
				Error( node->RetExpr()->Location(), "Return value type (" + rettype->ToString() + ") does not match function type (" + m_pCurrentFunc->Signature().ReturnType()->ToString() + ")" );
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
	void SemanticAnalysis::VisitBreakStmt( BreakStmt* node )
	{
		if( !m_pCurrentLoop )
		{
			Error( node->Location(), "Break statement used outside of loop" );
		}
	}
	void SemanticAnalysis::VisitContinueStmt( ContinueStmt* node )
	{
		if( !m_pCurrentLoop )
		{
			Error( node->Location(), "Continue statement used outside of loop" );
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
				Error( node->Location(), "Module '" + node->ModuleName() + "' not found" );
				return;
			}
		}

		ErrorSysSource err_source( filename );
		auto source = MemoryStream::FromFile( filename, FileMode::TEXT );

		Lexer l( source.Base() );
		auto tokens = l.Scan();

		if( ErrorSys::HadError() )
			return;

		Parser p( std::move( tokens ) );
		std::vector<std::unique_ptr<Statement>> res = p.Parse();

		if( ErrorSys::HadError() )
			return;

		SymbolTable* old_symtab = m_pSymTab;
		m_pSymTab = new SymbolTable();

		for( size_t i = 0; i < res.size(); i++ )
		{
			Analyze( res[i].get() );
			m_pStatements.push_back( std::move( res[i] ) );
		}

		SymbolTable* module_symtab = m_pSymTab;
		m_pSymTab = old_symtab;

		if( node->HasImports() )
		{
			const auto& imports = node->Imports();
			if( imports[0].type == TOKEN_ASTERISK )
			{
				if( !m_pSymTab->StealSymbolsFrom( *module_symtab ) )
				{
					Error( imports[0].loc, "Conflicting symbols" );
				}
			}
			else
			{
				for( const Token& import_name : imports )
				{
					std::unique_ptr<Symbol> sym = module_symtab->TakeSymbol( import_name.lexeme );
					if( !sym || sym->IsModule() )
					{
						Error( import_name.loc, "Symbol '" + import_name.lexeme + "' not found" );
						continue;
					}

					if( !m_pSymTab->AddSymbol( import_name.lexeme, std::move( sym ) ) )
					{
						Error( import_name.loc, "Symbol '" + import_name.lexeme + "' already defined" );
						continue;
					}
				}
			}

			delete module_symtab;
		}
		else
		{
			if( !m_pSymTab->AddSymbol( node->AsName(), std::make_unique<ModuleSymbol>( node, module_symtab ) ) )
			{
				Error( node->Location(), "Symbol '" + node->AsName() + "' already defined" );
			}
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

		if( sig.ReturnTypeSpec().HasTypeName() )
		{
			sig.SetReturnType( TypeSpecifierToType( sig.ReturnTypeSpec() ) );
		}
		else
		{
			sig.SetReturnType( typeman.NewPrimitive( PrimitiveKind::Void ) );
		}
	}
	void SemanticAnalysis::VisitVarDecl( VarDecl* node )
	{
		if( node->TypeSpec().HasTypeName() )
		{
			Type* var_type = TypeSpecifierToType( node->TypeSpec() );

			if( node->Initializer() )
			{
				Type* init_type = GetExprType( node->Initializer() );
				Type* coerced = Coerce( init_type, var_type );
				if( !coerced )
				{
					Error( node->Initializer()->Location(), "Cannot assign expression of type " + init_type->ToString() + " to variable of type " + var_type->ToString() );
				}
				else if( coerced != init_type )
				{
					auto cast = std::make_unique<CastExpr>( node->TakeInitializer(), var_type );
					node->SetInitializer( std::move( cast ) );
				}

				AddVariable( node, node->Identifier(), var_type );
			}
			else
			{
				AddVariable( node, node->Identifier(), var_type );
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
				AddVariable( node, node->Identifier(), init_type );

				node->SetType( init_type );
			}
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
			if( !sig.ParamDefault( i ) && !sig.ParamType( i ).HasTypeName() )
			{
				Error( sig.ParamType( i ).TypeName().loc, "Could not resolve variable type" );
			}
			else if( sig.ParamDefault( i ) && sig.ParamType( i ).HasTypeName() )
			{
				Type* param_type = TypeSpecifierToType( sig.ParamType( i ) );
				if( !param_type )
				{
					Error( sig.ParamType( i ).TypeName().loc, "Unknown type '" + sig.ParamType( i ).TypeName().lexeme + "'" );
					param_type = typeman.NewPrimitive( PrimitiveKind::Int );
				}

				Type* default_expr_type = GetExprType( sig.ParamDefault( i ) );
				Type* coerced = Coerce( default_expr_type, param_type );
				if( !coerced )
				{
					Error( sig.ParamDefault( i )->Location(), std::string( "Cannot assign expression of type " ) + default_expr_type->ToString() +
						" to parameter of type " + param_type->ToString() );
				}
				else if( coerced != default_expr_type )
				{
					auto cast = std::make_unique<CastExpr>( sig.TakeParamDefault( i ), param_type );
					sig.SetParamDefault( i, std::move( cast ) );
				}

				// Add the variable regardless of coercion success so that the errors don't pile up
				AddVariable( node, sig.ParamIdent( i ), param_type );
			}
			else if( sig.ParamDefault( i ) )
			{
				Type* default_expr_type = GetExprType( sig.ParamDefault( i ) );
				AddVariable( node, sig.ParamIdent( i ), default_expr_type );
			}
			else
			{
				Type* param_type = TypeSpecifierToType( sig.ParamType( i ) );
				if( !param_type )
				{
					Error( sig.ParamType( i ).TypeName().loc, "Unknown type '" + sig.ParamType( i ).TypeName().lexeme + "'" );
					param_type = typeman.NewPrimitive( PrimitiveKind::Int );
				}
				AddVariable( node, sig.ParamIdent( i ), param_type );
			}
		}

		if( sig.ReturnTypeSpec().HasTypeName() )
		{
			sig.SetReturnType( TypeSpecifierToType( sig.ReturnTypeSpec() ) );
		}

		Analyze( node->Body() );

		if( !node->Body()->IsBlockStmt() )
		{
			std::vector<std::unique_ptr<Statement>> body;
			body.push_back( node->TakeBody() );
			auto loc = body[0]->Location();
			node->SetBody( std::make_unique<BlockStmt>( loc, std::move( body ) ) );
		}

		// Couldn't deduce return type from returns because there weren't any
		// No returns = void function
		if( sig.ReturnType() == nullptr )
		{
			sig.SetReturnType( typeman.NewPrimitive( PrimitiveKind::Void ) );
		}

		auto body = node->Body()->AsBlockStmt();
		auto last_stmt = body->Stmt( body->NumStatements() - 1 );
		if( !last_stmt->IsReturnStmt() )
		{
			// Any non-void function must end in return
			if( !(sig.ReturnType()->IsPrimitive() &&
				sig.ReturnType()->AsPrimitive()->PrimKind() == PrimitiveKind::Void) )
			{
				Error( last_stmt->Location(), "Function missing return value" );
			}

			// Add a return here if it doesn't exist, compiler expects one
			body->Add( std::make_unique<ReturnStmt>( last_stmt->Location(), nullptr ) );
		}

		m_pCurrentFunc = nullptr;
		PopScope();
	}
	void SemanticAnalysis::VisitClassDecl( ClassDecl* node )
	{
		if( !InGlobalScope() )
		{
			Error( node->Location(), "Class must be declared in global scope" );
			return;
		}

		SymbolTable* old_symtab = m_pSymTab;
		SymbolTable* class_symtab = new SymbolTable( old_symtab );
		
		m_pSymTab = class_symtab;

		Type* type = typeman.NewClass( node, class_symtab );
		if( !type )
		{
			Error( node->Name().loc, "Type name already exists" );
			delete class_symtab;
			m_pSymTab = old_symtab;
			return;
		}

		for( const auto& field : node->Fields() )
			VisitVarDecl( field.get() );
		for( const auto& method : node->Methods() )
			VisitFuncDecl( method.get() );
		for( const auto& native : node->Natives() )
			VisitNativeStmt( native.get() );

		m_pSymTab = old_symtab;

		if( !m_pSymTab->AddSymbol( node->Name().lexeme, std::make_unique<ClassSymbol>( node, class_symtab ) ) )
		{
			Error( node->Name().loc, "Type symbol already exists" );
			return;
		}

		// TODO: Type check constructor method
	}
}
