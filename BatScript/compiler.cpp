#include "compiler.h"

#include <algorithm>
#include <fstream>
#include "errorsys.h"
#include "lexer.h"
#include "parser.h"
#include "type_manager.h"

namespace Bat
{
	static ObjectType TypeToObjectType( Type* t )
	{
		if( PrimitiveType* p = t->ToPrimitive() )
		{
			switch( p->PrimKind() )
			{
			case PrimitiveKind::Bool:
				return TYPE_BOOL;
			case PrimitiveKind::Int:
				return TYPE_INT;
			case PrimitiveKind::Float:
				return TYPE_FLOAT;
			case PrimitiveKind::String:
				return TYPE_STR;
			default:
				assert( false );
			}
		}

		if( FunctionType* f = t->ToFunction() )
		{
			return TYPE_CALLABLE;
		}

		if( ArrayType* a = t->ToArray() )
		{
			return TYPE_ARRAY;
		}

		return TYPE_UNDEFINED;
	}

	Compiler::Compiler()
	{
		m_pSymTab = new SymbolTable;
	}
	Compiler::~Compiler()
	{
		delete m_pSymTab;
	}
	void Compiler::Compile( std::vector<std::unique_ptr<Statement>> statements )
	{
		// Do imports first
		for( const auto& stmt : statements )
		{
			if( ImportStmt* impt = stmt->ToImportStmt() )
			{
				Compile( impt );
			}
		}

		m_iStackSize = 0;

		// Global vars second
		for( const auto& stmt : statements )
		{
			if( VarDecl* var = stmt->ToVarDecl() )
			{
				Compile( var );
			}
		}

		int globals_stack = m_iStackSize;

		// Functions third
		for( const auto& stmt : statements )
		{
			if( FuncDecl* func = stmt->ToFuncDecl() )
			{
				Compile( func );
			}
		}

		m_iEntryPoint = IP();

		Emit( OpCode::PROC );
		if( globals_stack > 0 )
		{
			Emit( OpCode::STACK, globals_stack );
		}

		// Initialize all the global variables
		CompileGlobalInitializers();

		// Everything else gets put into a pseudo-function as the mainline
		for( const auto& stmt : statements )
		{
			if( !stmt->IsImportStmt() && !stmt->IsFuncDecl() && !stmt->IsVarDecl() )
			{
				Compile( stmt.get() );
			}
		}

		Emit( OpCode::ENDPROC, globals_stack );
		Emit( OpCode::HALT );
	}
	void Compiler::Compile( std::unique_ptr<Statement> s )
	{
		Compile( s.get() );
		m_pStatements.push_back( std::move( s ) );
	}
	void Compiler::Compile( Statement* s )
	{
		s->Accept( this );

		// Expressions push 1 value onto stack, but statements should have no effect on stack
		// So if this statement just evaluates an expression, pop off the unused value
		if( s->IsExpressionStmt() )
		{
			Emit( OpCode::POP );
		}
	}
	void Compiler::CompileLValue( Expression* e )
	{
		if( !e->IsLValue() )
		{
			ErrorSys::Report( e->Location().Line(), e->Location().Column(), "Not a modifiable lvalue" );
			return;
		}

		m_CompileType = ExprType::LVALUE;
		e->Accept( this );
		m_CompileType = ExprType::UNKNOWN;
	}
	void Compiler::CompileRValue( Expression* e )
	{
		m_CompileType = ExprType::RVALUE;
		e->Accept( this );
		m_CompileType = ExprType::UNKNOWN;
	}
	CodeLoc_t Compiler::Emit( OpCode op )
	{
		m_LineMapping.push_back( m_iCurrentLine );

		auto loc = IP();
		code.Write( op );
		return loc;
	}
	CodeLoc_t Compiler::Emit( OpCode op, int64_t param1 )
	{
		auto loc = Emit( op );
		EmitI64( param1 );
		return loc;
	}
	CodeLoc_t Compiler::EmitF( OpCode op, double param1 )
	{
		auto loc = Emit( op );
		EmitDouble( param1 );
		return loc;
	}
	CodeLoc_t Compiler::EmitByte( char byte )
	{
		auto loc = IP();
		code.WriteByte( byte );
		return loc;
	}
	CodeLoc_t Compiler::EmitI16( int16_t i16 )
	{
		auto loc = IP();
		code.WriteInt16( i16 );
		return loc;
	}
	CodeLoc_t Compiler::EmitI32( int32_t i32 )
	{
		auto loc = IP();
		code.WriteInt32( i32 );
		return loc;
	}
	CodeLoc_t Compiler::EmitI64( int64_t i64 )
	{
		auto loc = IP();
		code.WriteInt64( i64 );
		return loc;
	}
	CodeLoc_t Compiler::EmitU16( uint16_t u16 )
	{
		auto loc = IP();
		code.WriteUInt16( u16 );
		return loc;
	}
	CodeLoc_t Compiler::EmitU32( uint32_t u32 )
	{
		auto loc = IP();
		code.WriteUInt32( u32 );
		return loc;
	}
	CodeLoc_t Compiler::EmitU64( uint64_t u64 )
	{
		auto loc = IP();
		code.WriteUInt64( u64 );
		return loc;
	}
	CodeLoc_t Compiler::EmitFloat( float f )
	{
		auto loc = IP();
		code.WriteFloat( f );
		return loc;
	}
	CodeLoc_t Compiler::EmitDouble( double d )
	{
		auto loc = IP();
		code.WriteDouble( d );
		return loc;
	}
	CodeLoc_t Compiler::EmitToPatch( OpCode op )
	{
		return Emit( op, 0 );
	}
	void Compiler::Patch( CodeLoc_t addr, int64_t value )
	{
		auto old = code.Tell();
		code.Seek( (size_t)addr + sizeof( OpCode ), SeekPosition::START );
		code.Write( value );
		code.Seek( old, SeekPosition::START );
	}
	void Compiler::PatchJump( CodeLoc_t addr )
	{
		Patch( addr, IP() );
	}
	void Compiler::EmitReturn()
	{
		CodeLoc_t func_end = EmitToPatch( OpCode::JMP );
		m_ReturnsToPatch.push_back( func_end );
	}
	void Compiler::PatchReturns()
	{
		for( CodeLoc_t ret : m_ReturnsToPatch )
		{
			PatchJump( ret );
		}
		m_ReturnsToPatch.clear();
	}
	void Compiler::EmitLoad( Symbol* sym )
	{
		if( VariableSymbol* var = sym->AsVariable() )
		{
			bool global = (var->Storage() == StorageClass::GLOBAL);
			if( global )
			{
				Emit( OpCode::LOAD_GLOBAL );
			}
			else
			{
				Emit( OpCode::LOAD_LOCAL );
			}
		}
		else
		{
			assert( false );
		}
	}
	void Compiler::EmitStore( Symbol* sym )
	{
		if( VariableSymbol * var = sym->AsVariable() )
		{
			bool global = (var->Storage() == StorageClass::GLOBAL);
			if( global )
			{
				Emit( OpCode::STORE_GLOBAL );
			}
			else
			{
				Emit( OpCode::STORE_LOCAL );
			}
		}
		else
		{
			assert( false );
		}
	}
	BatCode Compiler::Code() const
	{
		BatCode bc;
		bc.code = code;
		bc.code.Seek( SeekPosition::START );
		bc.entry_point = m_iEntryPoint;
		bc.string_literals = m_StringLiterals;
		bc.debug_info.line_mapping = m_LineMapping;

		for( size_t i = 0; i < m_Natives.size(); i++ )
		{
			BatNativeInfo info;
			info.name = m_Natives[i];

			FunctionSymbol* ntv = GetSymbol( info.name )->AsFunction();
			FunctionSignature& sig = ntv->Signature();
			for( size_t param_idx = 0; param_idx < sig.NumParams(); i++ )
			{
				Type* t = TypeSpecifierToType( sig.ParamType( param_idx ) );
				ObjectType obj_type = TypeToObjectType( t );

				info.desc.param_types.push_back( obj_type );
			}

			bc.natives.push_back( info );
		}

		return bc;
	}
	void Compiler::CompileBinaryRValue( BinaryExpr* node )
	{
		CompileRValue( node->Right() );
		CompileRValue( node->Left() );

		PrimitiveKind kind = node->Left()->Type()->ToPrimitive()->PrimKind();

		switch( node->Op() )
		{
		case TOKEN_BAR:             Emit( OpCode::BITOR ); break;
		case TOKEN_HAT:             Emit( OpCode::BITXOR ); break;
		case TOKEN_AMP:             Emit( OpCode::BITAND ); break;
		case TOKEN_LESS_LESS:       Emit( OpCode::SHL ); break;
		case TOKEN_GREATER_GREATER: Emit( OpCode::SHR ); break;
		case TOKEN_EQUAL_EQUAL:     (kind == PrimitiveKind::Int) ? Emit( OpCode::EQ ) : Emit( OpCode::EQF ); break;
		case TOKEN_EXCLMARK_EQUAL:  (kind == PrimitiveKind::Int) ? Emit( OpCode::NEQ ) : Emit( OpCode::NEQF ); break;
		case TOKEN_LESS:            (kind == PrimitiveKind::Int) ? Emit( OpCode::LESS ) : Emit( OpCode::LESSF ); break;
		case TOKEN_LESS_EQUAL:      (kind == PrimitiveKind::Int) ? Emit( OpCode::LESSE ) : Emit( OpCode::LESSEF ); break;
		case TOKEN_GREATER:         (kind == PrimitiveKind::Int) ? Emit( OpCode::GRT ) : Emit( OpCode::GRTF ); break;
		case TOKEN_GREATER_EQUAL:   (kind == PrimitiveKind::Int) ? Emit( OpCode::GRTE ) : Emit( OpCode::GRTEF ); break;
		case TOKEN_PLUS:            (kind == PrimitiveKind::Int) ? Emit( OpCode::ADD ) : Emit( OpCode::ADDF ); break;
		case TOKEN_MINUS:           (kind == PrimitiveKind::Int) ? Emit( OpCode::SUB ) : Emit( OpCode::SUBF ); break;
		case TOKEN_ASTERISK:        (kind == PrimitiveKind::Int) ? Emit( OpCode::MUL ) : Emit( OpCode::MULF ); break;
		case TOKEN_SLASH:           (kind == PrimitiveKind::Int) ? Emit( OpCode::DIV ) : Emit( OpCode::DIVF ); break;
		case TOKEN_PERCENT:         Emit( OpCode::MOD ); break;
		}
	}
	void Compiler::CompileBinaryLValue( BinaryExpr* node )
	{
		Symbol* sym = GetSymbol( node->Left() );

		/* Straight assignment is a simple store operation */
		if( node->Op() == TOKEN_EQUAL )
		{
			CompileLValue( node->Left() );
			CompileRValue( node->Right() );
			EmitStore( sym );
			return;
		}

		CompileRValue( node->Right() );
		CompileLValue( node->Left() );

		/* Compound assignment operations are a little more involved ... */

		//  dupx1  ; stack: [addr, value]
		//  load   ; stack: [addr, value, addr]
		//  add    ; stack: [loaded, value, addr]
		//  store  ; stack: [added, addr]
		//  ; ...    stack: []

		PrimitiveKind kind = node->Type()->ToPrimitive()->PrimKind();

		Emit( OpCode::DUPX1 );
		EmitLoad( sym );

		switch( node->Op() )
		{
		case TOKEN_PLUS_EQUAL:     (kind == PrimitiveKind::Int) ? Emit( OpCode::ADD ) : Emit( OpCode::ADDF ); break;
		case TOKEN_MINUS_EQUAL:    (kind == PrimitiveKind::Int) ? Emit( OpCode::SUB ) : Emit( OpCode::SUBF ); break;
		case TOKEN_ASTERISK_EQUAL: (kind == PrimitiveKind::Int) ? Emit( OpCode::MUL ) : Emit( OpCode::MULF ); break;
		case TOKEN_SLASH_EQUAL:    (kind == PrimitiveKind::Int) ? Emit( OpCode::DIV ) : Emit( OpCode::DIVF ); break;
		case TOKEN_PERCENT_EQUAL:  (kind == PrimitiveKind::Int) ? Emit( OpCode::MOD ) : Emit( OpCode::POP ); break;
		case TOKEN_AMP_EQUAL:      Emit( OpCode::BITAND ); break;
		case TOKEN_HAT_EQUAL:      Emit( OpCode::BITXOR ); break;
		case TOKEN_BAR_EQUAL:      Emit( OpCode::BITOR ); break;
		}

		EmitStore( sym );
	}
	VariableSymbol* Compiler::AddVariable( AstNode* node, const std::string& name, StorageClass storage, Type* type )
	{
		m_pSymTab->AddSymbol( name, std::make_unique<VariableSymbol>( node, type ) );
		VariableSymbol* var = m_pSymTab->GetSymbol( name )->ToVariable();
		var->SetStorage( storage );
		return var;
	}
	Symbol* Compiler::GetSymbol( const std::string& name ) const
	{
		return m_pSymTab->GetSymbol( name );
	}
	Symbol* Compiler::GetSymbol( Expression* node ) const
	{
		assert( node->IsLValue() );
		if( VarExpr* var = node->AsVarExpr() )
		{
			return GetSymbol( var->Identifier().lexeme );
		}

		assert( false );
		return nullptr;
	}
	FunctionSymbol* Compiler::AddFunction( AstNode* node, const std::string& name )
	{
		m_pSymTab->AddSymbol( name, std::make_unique<FunctionSymbol>( node, FunctionKind::Script ) );
		FunctionSymbol* func = m_pSymTab->GetSymbol( name )->ToFunction();
		func->SetAddress( IP() );
		return func;
	}
	FunctionSymbol* Compiler::AddNative( NativeStmt* node, const std::string& name )
	{
		m_pSymTab->AddSymbol( name, std::make_unique<FunctionSymbol>( node, FunctionKind::Native ) );
		FunctionSymbol* ntv = m_pSymTab->GetSymbol( name )->ToFunction();
		auto& sig = ntv->Signature();
		sig.SetReturnType( TypeSpecifierToType( sig.ReturnTypeSpec() ) ); // HACK: imported natives dont get passed to us from sema pass, so they don't have their return type filled in.
		ntv->SetAddress( m_Natives.size() );
		m_Natives.push_back( name );
		return ntv;
	}
	void Compiler::AddGlobalInitializer( Symbol* var, Expression* initializer )
	{
		GlobalInitializer gi;
		gi.var = var;
		gi.initializer = initializer;
		m_GlobalInitializers.push_back( gi );
	}
	void Compiler::CompileGlobalInitializers()
	{
		for( const GlobalInitializer& gi : m_GlobalInitializers )
		{
			Emit( OpCode::PUSH, gi.var->Address() );
			CompileRValue( gi.initializer );
			EmitStore( gi.var );
			Emit( OpCode::POP );
		}
	}
	void Compiler::PushScope()
	{
		m_pSymTab = new SymbolTable( m_pSymTab );
	}
	void Compiler::PopScope()
	{
		assert( m_pSymTab->Enclosing() != nullptr );
		SymbolTable* temp = m_pSymTab;
		m_pSymTab = m_pSymTab->Enclosing();
		delete temp;
	}
	int64_t Compiler::AddStringLiteral( const std::string& literal )
	{
		for( size_t i = 0; i < m_StringLiterals.size(); i++ )
		{
			if( m_StringLiterals[i] == literal )
			{
				return (int64_t)i;
			}
		}

		auto idx = (int64_t)m_StringLiterals.size();
		m_StringLiterals.push_back( literal );
		return idx;
	}
	void Compiler::UpdateCurrLine( AstNode* node )
	{
		m_iCurrentLine = node->Location().Line();
	}
	void Compiler::VisitIntLiteral( IntLiteral* node )
	{
		UpdateCurrLine( node );

		Emit( OpCode::PUSH, node->value );
	}
	void Compiler::VisitFloatLiteral( FloatLiteral* node )
	{
		UpdateCurrLine( node );

		EmitF( OpCode::PUSH, node->value );
	}
	void Compiler::VisitStringLiteral( StringLiteral* node )
	{
		UpdateCurrLine( node );

		int64_t index = AddStringLiteral( node->value );
		Emit( OpCode::PUSH, index );
	}
	void Compiler::VisitTokenLiteral( TokenLiteral* node )
	{
		UpdateCurrLine( node );

		switch( node->value )
		{
		case TOKEN_TRUE:
			Emit( OpCode::PUSH, 1 );
			break;
		case TOKEN_FALSE:
			Emit( OpCode::PUSH, 0 );
			break;
		case TOKEN_NIL:
			Emit( OpCode::PUSH, 0 );
			break;
		default:
			assert( false && "Unhandled token literal" );
			Emit( OpCode::PUSH, 0 );
			break;
		}
	}
	void Compiler::VisitArrayLiteral( ArrayLiteral* node )
	{
		UpdateCurrLine( node );

		assert( false );
	}
	void Compiler::VisitBinaryExpr( BinaryExpr* node )
	{
		UpdateCurrLine( node );

		// Logical operators like and/or have short-circuiting, so they have to be handled a little different
		if( node->Op() == TOKEN_AND )
		{
			//  ; left expression
			//  jz early_exit
			//  ; right expression
			//  jmp end
			// early_exit:
			//  push 0
			// end:
			//  ; ...

			CompileRValue( node->Left() );
			CodeLoc_t early_exit_target = EmitToPatch( OpCode::JZ );
			CompileRValue( node->Right() );
			CodeLoc_t end_target = EmitToPatch( OpCode::JMP );
			PatchJump( early_exit_target );
			Emit( OpCode::PUSH, 0 );
			PatchJump( end_target );
			return;
		}
		else if( node->Op() == TOKEN_OR )
		{
			//  ; left expression
			//  jnz early_exit
			//  ; right expression
			//  jmp end
			// early_exit:
			//  push 1
			// end:
			//  ; ...

			CompileRValue( node->Left() );
			CodeLoc_t early_exit_target = EmitToPatch( OpCode::JNZ );
			CompileRValue( node->Right() );
			CodeLoc_t end_target = EmitToPatch( OpCode::JMP );
			PatchJump( early_exit_target );
			Emit( OpCode::PUSH, 1 );
			PatchJump( end_target );
			return;
		}

		switch( node->Op() )
		{
		case TOKEN_BAR:
		case TOKEN_HAT:
		case TOKEN_AMP:
		case TOKEN_LESS_LESS:
		case TOKEN_GREATER_GREATER:
		case TOKEN_EQUAL_EQUAL:
		case TOKEN_EXCLMARK_EQUAL:
		case TOKEN_LESS:
		case TOKEN_LESS_EQUAL:
		case TOKEN_GREATER:
		case TOKEN_GREATER_EQUAL:
		case TOKEN_PLUS:
		case TOKEN_MINUS:
		case TOKEN_ASTERISK:
		case TOKEN_SLASH:
		case TOKEN_PERCENT:
		{
			CompileBinaryRValue( node );
			break;
		}

		case TOKEN_EQUAL:
		case TOKEN_PLUS_EQUAL:
		case TOKEN_MINUS_EQUAL:
		case TOKEN_ASTERISK_EQUAL:
		case TOKEN_SLASH_EQUAL:
		case TOKEN_PERCENT_EQUAL:
		case TOKEN_AMP_EQUAL:
		case TOKEN_HAT_EQUAL:
		case TOKEN_BAR_EQUAL:
		{
			CompileBinaryLValue( node );
			break;
		}

		default:
			assert( false && "Unhandled binary op" );
		}
	}
	void Compiler::VisitUnaryExpr( UnaryExpr* node )
	{
		UpdateCurrLine( node );

		CompileRValue( node->Right() );

		PrimitiveKind kind = node->Type()->ToPrimitive()->PrimKind();

		switch( node->Op() )
		{
		case TOKEN_MINUS:    (kind == PrimitiveKind::Int) ? Emit( OpCode::NEG ) : Emit( OpCode::NEGF ); break;
		case TOKEN_EXCLMARK: Emit( OpCode::NOT ); break;
		case TOKEN_TILDE:    Emit( OpCode::BITNOT ); break;
		default:
			assert( false && "Unhandled unary op" );
		}
	}
	void Compiler::VisitCallExpr( CallExpr* node )
	{
		UpdateCurrLine( node );

		VarExpr* callee = node->Function()->ToVarExpr();
		Symbol* symbol = m_pSymTab->GetSymbol( callee->Identifier().lexeme );

		FunctionSymbol* func_symbol = symbol->ToFunction();

		auto& sig = func_symbol->Signature();
		
		// Reserve space for return value
		if( func_symbol->FuncKind() == FunctionKind::Script )
		{
			Emit( OpCode::PUSH, 0 );
		}

		for( size_t i = 0; i < node->NumArgs(); i++ )
		{
			CompileRValue( node->Arg( i ) );
		}

		CompileLValue( node->Function() );
		if( func_symbol->FuncKind() == FunctionKind::Script )
		{
			Emit( OpCode::CALL );
		}
		else if( func_symbol->FuncKind() == FunctionKind::Native )
		{
			Emit( OpCode::NATIVE );
		}
	}
	void Compiler::VisitIndexExpr( IndexExpr* node )
	{
		UpdateCurrLine( node );

		CompileLValue( node->Array() );
		CompileRValue( node->Index() );
		Emit( OpCode::ADD );

		assert( false );
	}
	void Compiler::VisitCastExpr( CastExpr* node )
	{
		UpdateCurrLine( node );

		Expression* base = node->Expr();
		PrimitiveType* base_type = base->Type()->ToPrimitive();
		PrimitiveType* target = node->TargetType()->ToPrimitive();

		// Only casting between primitives is supported for now
		assert( base_type && target );

		CompileRValue( base );

		if( base_type->PrimKind() == PrimitiveKind::Int && target->PrimKind() == PrimitiveKind::Float )
		{
			Emit( OpCode::ITOF );
		}
		else if( base_type->PrimKind() == PrimitiveKind::Float && target->PrimKind() == PrimitiveKind::Int )
		{
			Emit( OpCode::FTOI );
		}
		else if( base_type->PrimKind() == PrimitiveKind::Int && target->PrimKind() == PrimitiveKind::Bool )
		{
			Emit( OpCode::NOT );
			Emit( OpCode::NOT );
		}
		else if( base_type->PrimKind() == PrimitiveKind::Float && target->PrimKind() == PrimitiveKind::Bool )
		{
			Emit( OpCode::NOT );
			Emit( OpCode::NOT );
		}
		else
		{
			assert( false && "Unhandled cast type" );
		}
	}
	void Compiler::VisitGroupExpr( GroupExpr* node )
	{
		UpdateCurrLine( node );

		if( m_CompileType == ExprType::LVALUE )
		{
			CompileLValue( node->Expr() );
		}
		else if( m_CompileType == ExprType::RVALUE )
		{
			CompileRValue( node->Expr() );
		}
	}
	void Compiler::VisitVarExpr( VarExpr* node )
	{
		UpdateCurrLine( node );

		assert( m_CompileType != ExprType::UNKNOWN );

		Symbol* sym = GetSymbol( node->Identifier().lexeme );
		Emit( OpCode::PUSH, sym->Address() );
		if( m_CompileType == ExprType::RVALUE )
		{
			EmitLoad( sym );
		}
	}
	void Compiler::VisitExpressionStmt( ExpressionStmt* node )
	{
		UpdateCurrLine( node );

		CompileRValue( node->Expr() );
	}
	void Compiler::VisitBlockStmt( BlockStmt* node )
	{
		UpdateCurrLine( node );

		PushScope();
		size_t count = node->NumStatements();
		for( size_t i = 0; i < count; i++ )
		{
			Compile( node->Stmt( i ) );
		}
		PopScope();
	}
	void Compiler::VisitPrintStmt( PrintStmt* node )
	{
		UpdateCurrLine( node );

		CompileRValue( node->Expr() );

		PrimitiveType* t = node->Expr()->Type()->ToPrimitive();
		assert( t );

		switch( t->PrimKind() )
		{
		case PrimitiveKind::Bool:
			Emit( OpCode::PRINTB );
			break;
		case PrimitiveKind::Int:
			Emit( OpCode::PRINTI );
			break;
		case PrimitiveKind::Float:
			Emit( OpCode::PRINTF );
			break;
		case PrimitiveKind::String:
			Emit( OpCode::PRINTS );
			break;
		}
	}
	void Compiler::VisitIfStmt( IfStmt* node )
	{
		UpdateCurrLine( node );

		/* When else statement exists */
		//  ; condition
		//  jz else_branch
		//  ; then branch body
		//  jmp end
		// else_branch:
		//  ; else branch body
		// end:
		//  ; ....
		//
		/* When else statement does not exist */
		//  ; condition
		//  jz end
		//  ; then branch body
		// end:
		//  ; ...

		CompileRValue( node->Condition() );

		if( node->Else() )
		{
			CodeLoc_t else_target = EmitToPatch( OpCode::JZ );
			Compile( node->Then() );
			CodeLoc_t end_target = EmitToPatch( OpCode::JMP );
			PatchJump( else_target );
			Compile( node->Else() );
			PatchJump( end_target );
		}
		else
		{
			CodeLoc_t end_target = EmitToPatch( OpCode::JZ );
			Compile( node->Then() );
			PatchJump( end_target );
		}
	}
	void Compiler::VisitWhileStmt( WhileStmt* node )
	{
		UpdateCurrLine( node );

		// check:
		//  ; condition
		//  jz out
		//  ; while body
		//  jmp check
		// out:
		//  ; ...

		CodeLoc_t check_addr = IP();
		CompileRValue( node->Condition() );
		CodeLoc_t out_patch = EmitToPatch( OpCode::JZ );
		Compile( node->Body() );
		Emit( OpCode::JMP, check_addr );

		PatchJump( out_patch );
	}
	void Compiler::VisitForStmt( ForStmt* node )
	{
		UpdateCurrLine( node );

		assert( false );
	}
	void Compiler::VisitReturnStmt( ReturnStmt* node )
	{
		UpdateCurrLine( node );

		if( node->RetExpr() )
		{
			Emit( OpCode::PUSH, m_iRetAddr );
			CompileRValue( node->RetExpr() );
			Emit( OpCode::STORE_LOCAL );
			Emit( OpCode::POP );
			EmitReturn();
		}
	}
	void Compiler::VisitImportStmt( ImportStmt* node )
	{
		UpdateCurrLine( node );

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
			Compile( std::move( res[i] ) );
		}
	}
	void Compiler::VisitNativeStmt( NativeStmt* node )
	{
		UpdateCurrLine( node );

		AddNative( node, node->Signature().Identifier().lexeme );
	}
	void Compiler::VisitVarDecl( VarDecl* node )
	{
		UpdateCurrLine( node );

		VariableSymbol* var = AddVariable( node, node->Identifier().lexeme, InGlobalScope() ? StorageClass::GLOBAL : StorageClass::LOCAL, node->Type() );
		var->SetAddress( m_iStackSize );

		m_iStackSize += (int)node->Type()->Size();

		if( node->Initializer() )
		{
			if( InGlobalScope() )
			{
				// Globals have to be initialized at start of program
				AddGlobalInitializer( var, node->Initializer() );
			}
			else
			{
				// Non-globals can be initialized where they're declared
				Emit( OpCode::PUSH, var->Address() );
				CompileRValue( node->Initializer() );
				EmitStore( var );
				Emit( OpCode::POP );
			}
		}
	}
	void Compiler::VisitFuncDecl( FuncDecl* node )
	{
		UpdateCurrLine( node );

		auto& sig = node->Signature();
		AddFunction( node, sig.Identifier().lexeme );

		m_iStackSize = 0;
		
		//  proc
		//  stack XX
		//  ; function body
		//  endproc

		Emit( OpCode::PROC );
		CodeLoc_t stack_size = EmitToPatch( OpCode::STACK );

		constexpr int64_t arg_size = (int64_t)sizeof( int64_t );
		constexpr int64_t first_arg_addr = -1 * arg_size;;
		const int64_t last_arg_addr = (first_arg_addr - (sig.NumParams() - 1) * arg_size);
		m_iRetAddr = last_arg_addr - arg_size; // Return value is a hidden pseudo-parameter

		PushScope();

		size_t args_size = 0;
		for( size_t i = 0; i < sig.NumParams(); i++ )
		{
			Type* arg_type = TypeSpecifierToType( sig.ParamType( i ) );
			VariableSymbol* arg = AddVariable( node, sig.ParamIdent( i ).lexeme, StorageClass::ARGUMENT, arg_type );
			arg->SetAddress( last_arg_addr + i * sizeof( int64_t ) );
			args_size += arg_type->Size();

			// TODO: defaults are handled wrong, should be handled at caller side
			if( sig.ParamDefault( i ) )
			{
				Emit( OpCode::PUSH, arg->Address() );
				CompileRValue( sig.ParamDefault( i ) );
				EmitStore( arg );
				Emit( OpCode::POP );
			}
		}

		Compile( node->Body() );

		Patch( stack_size, m_iStackSize );

		// All the returns in the function just jump to here where cleanup is done, patch their target address
		PatchReturns();

		Emit( OpCode::ENDPROC, args_size );
		Emit( OpCode::RET );

		PopScope();
	}
}
