#include "interpreter.h"

#include <iostream>
#include <fstream>
#include "bat_callable.h"
#include "runtime_error.h"
#include "lexer.h"
#include "parser.h"
#include "semantic_analysis.h"
#include "memory_stream.h"

#define BAT_RETURN( value ) do { m_Result = (value); return; } while( false )

namespace Bat
{
	// Safe way of saving environment and restoring at end of scope
	class EnvironmentRestore
	{
	public:
		EnvironmentRestore( Environment** to )
			:
			to( to ),
			value( *to )
		{}
		~EnvironmentRestore()
		{
			*to = value;
		}
	private:
		Environment** to;
		Environment* value;
	};

	Interpreter::Interpreter()
	{
		m_pEnvironment = new Environment( "__global__" );
	}
	Interpreter::~Interpreter()
	{
		delete m_pEnvironment;
	}
	BatObject Interpreter::Evaluate( Expression* e )
	{
		e->Accept( this );
		return m_Result;

		return BatObject();
	}

	void Interpreter::Execute( std::unique_ptr<Statement> s )
	{
		Execute( s.get() );
		m_pStatements.push_back( std::move( s ) );
	}

	void Interpreter::Execute( Statement* s )
	{
		s->Accept( this );
	}

	void Interpreter::ExecuteBlock( Statement* s, Environment& environment )
	{
		EnvironmentRestore save( &m_pEnvironment );
		m_pEnvironment = &environment;

		Execute( s );
	}

	void Interpreter::Call( const std::string& func_name, const std::vector<BatObject>& args )
	{
		try
		{
			const BatObject* obj = m_pEnvironment->GetVar( func_name );
			if( !obj )
				throw RuntimeError( "Function '" + func_name + "' not found" );
			
			if( obj->type != TYPE_CALLABLE )
			{
				throw RuntimeError( "Object '" + func_name + "' is not callable" );
			}

			BatObject func = *obj;
			func.Call( *this, args );
		}
		catch( const BatObjectError& e )
		{
			throw RuntimeError( e.what() );
		}
	}

	void Interpreter::AddModule( const std::string& name, BatModule module )
	{
		assert( FindModule( name ) == nullptr );
		m_Modules[name] = std::move( module );
	}

	void Interpreter::AddVar( const std::string& name, const BatObject& value, const SourceLoc& loc )
	{
		if( !m_pEnvironment->AddVar( name, value ) )
		{
			throw RuntimeError( loc, name + " is already defined in this scope" );
		}
	}

	void Interpreter::SetVar( const std::string& name, const BatObject& value, const SourceLoc& loc )
	{
		if( !m_pEnvironment->SetVar( name, value ) )
		{
			throw RuntimeError( loc, name + " is not defined" );
		}
	}

	const BatObject& Interpreter::GetVar( const std::string& name, const SourceLoc& loc )
	{
		const BatObject* obj = m_pEnvironment->GetVar( name );
		if( !obj )
		{
			throw RuntimeError( loc, name + " is not defined" );
		}

		return *obj;
	}

	bool Interpreter::IsTruthy( const BatObject& obj, const SourceLoc& loc )
	{
		try
		{
			return obj.IsTruthy();
		}
		catch( const BatObjectError& e )
		{
			throw RuntimeError( loc, e.what() );
		}
	}

	const BatModule* Interpreter::FindModule( const std::string& name ) const
	{
		auto it = m_Modules.find( name );
		if( it == m_Modules.end() )
			return nullptr;
		return &it->second;
	}

	void Interpreter::VisitIntLiteral( IntLiteral* node )
	{
		BAT_RETURN( BatObject::Int( node->value ) );
	}
	void Interpreter::VisitFloatLiteral( FloatLiteral* node )
	{
		BAT_RETURN( BatObject::Float( node->value ) );
	}
	void Interpreter::VisitStringLiteral( StringLiteral* node )
	{
		BAT_RETURN( BatObject::String( node->value ) );
	}
	void Interpreter::VisitTokenLiteral( TokenLiteral* node )
	{
		switch( node->value )
		{
			case TOKEN_TRUE:  BAT_RETURN( BatObject::True() );
			case TOKEN_FALSE: BAT_RETURN( BatObject::False() );
			case TOKEN_NIL:   BAT_RETURN( BatObject() );
			default:
				throw RuntimeError( node->Location(), std::string( "Unexpected token '" ) + TokenTypeToString( node->value ) + "'" );
		}
	}
	void Interpreter::VisitArrayLiteral( ArrayLiteral* node )
	{
		auto arr = std::make_unique<BatObject[]>( node->NumValues() );
		for( size_t i = 0; i < node->NumValues(); i++ )
		{
			arr[i] = Evaluate( node->ValueAt( i ) );
		}

		BatObject obj = BatObject::Array( arr.get(), node->NumValues() );

		BAT_RETURN( obj );
	}
	void Interpreter::VisitBinaryExpr( BinaryExpr* node )
	{
		try
		{
			Expression* l = node->Left();
			Expression* r = node->Right();
			switch( node->Op() )
			{
			case TOKEN_BAR:              BAT_RETURN( Evaluate( l ).BitOr( Evaluate( r ) ) );
			case TOKEN_HAT:              BAT_RETURN( Evaluate( l ).BitXor( Evaluate( r ) ) );
			case TOKEN_AMP:              BAT_RETURN( Evaluate( l ).BitAnd( Evaluate( r ) ) );
			case TOKEN_EQUAL_EQUAL:      BAT_RETURN( Evaluate( l ).CmpEq( Evaluate( r ) ) );
			case TOKEN_EXCLMARK_EQUAL:   BAT_RETURN( Evaluate( l ).CmpNeq( Evaluate( r ) ) );
			case TOKEN_LESS:             BAT_RETURN( Evaluate( l ).CmpL( Evaluate( r ) ) );
			case TOKEN_LESS_EQUAL:       BAT_RETURN( Evaluate( l ).CmpLe( Evaluate( r ) ) );
			case TOKEN_GREATER:          BAT_RETURN( Evaluate( l ).CmpG( Evaluate( r ) ) );
			case TOKEN_GREATER_EQUAL:    BAT_RETURN( Evaluate( l ).CmpGe( Evaluate( r ) ) );
			case TOKEN_LESS_LESS:        BAT_RETURN( Evaluate( l ).LShift( Evaluate( r ) ) );
			case TOKEN_GREATER_GREATER:  BAT_RETURN( Evaluate( l ).RShift( Evaluate( r ) ) );
			case TOKEN_PLUS:             BAT_RETURN( Evaluate( l ).Add( Evaluate( r ) ) );
			case TOKEN_MINUS:            BAT_RETURN( Evaluate( l ).Sub( Evaluate( r ) ) );
			case TOKEN_ASTERISK:         BAT_RETURN( Evaluate( l ).Mul( Evaluate( r ) ) );
			case TOKEN_SLASH:            BAT_RETURN( Evaluate( l ).Div( Evaluate( r ) ) );
			case TOKEN_PERCENT:          BAT_RETURN( Evaluate( l ).Mod( Evaluate( r ) ) );

			case TOKEN_OR:
				if( Evaluate( l ).IsTruthy() ) BAT_RETURN( BatObject::True() );
				if( Evaluate( r ).IsTruthy() ) BAT_RETURN( BatObject::True() );
				BAT_RETURN( BatObject::False() );
			case TOKEN_AND:
				if( !Evaluate( l ).IsTruthy() ) BAT_RETURN( BatObject::False() );
				if( !Evaluate( r ).IsTruthy() ) BAT_RETURN( BatObject::False() );
				BAT_RETURN( BatObject::True() );
			default:
				throw RuntimeError( node->Location(), "Unexpected binary expression" );
			}
		}
		catch( const BatObjectError& e )
		{
			throw RuntimeError( node->Location(), e.what() );
		}
	}
	void Interpreter::VisitUnaryExpr( UnaryExpr* node )
	{
		try
		{
			switch( node->Op() )
			{
				case TOKEN_MINUS:    BAT_RETURN( Evaluate( node->Right() ).Neg() );
				case TOKEN_EXCLMARK: BAT_RETURN( Evaluate( node->Right() ).Not() );
				case TOKEN_TILDE:    BAT_RETURN( Evaluate( node->Right() ).BitNeg() );
					// case TOKEN_AMP:
				default:
					throw RuntimeError( node->Location(), "Unexpected binary expression" );
			}
		}
		catch( const BatObjectError& e )
		{
			throw RuntimeError( node->Location(), e.what() );
		}
	}
	void Interpreter::VisitCallExpr( CallExpr* node )
	{
		try
		{
			BatObject func = Evaluate( node->Function() );
			size_t num_args = node->NumArgs();
			std::vector<BatObject> arguments;

			if( AttrExpr* attr = node->Function()->ToAttrExpr() )
			{
				BatObject base = Evaluate( attr->Base() );
				if( base.type == TYPE_OBJECT )
				{
					arguments.push_back( std::move( base ) );
				}
			}

			for( size_t i = 0; i < num_args; i++ )
			{
				arguments.push_back( Evaluate( node->Arg( i ) ) );
			}

			BAT_RETURN( func.Call( *this, arguments ) );
		}
		catch( const BatObjectError& e )
		{
			throw RuntimeError( node->Location(), e.what() );
		}
	}
	void Interpreter::VisitIndexExpr( IndexExpr* node )
	{
		try
		{
			BatObject arr = Evaluate( node->Array() );
			BatObject index = Evaluate( node->Index() );

			BAT_RETURN( arr.Index( index ) );
		}
		catch( const BatObjectError& e )
		{
			throw RuntimeError( node->Location(), e.what() );
		}
	}
	void Interpreter::VisitAttrExpr( AttrExpr* node )
	{
		BatObject base = Evaluate( node->Base() );
		if( base.type == TYPE_MODULE )
		{
			Environment* env = base.GetModule();

			const BatObject* var = env->GetVar( node->Attr().lexeme );
			assert( var );
			
			BAT_RETURN( *var );
		}
		else if( base.type == TYPE_OBJECT )
		{
			const BatObject& base_class = GetVar( base.classname, node->Location() );

			const BatObject* var = base_class.GetClass()->GetVar( node->Attr().lexeme );
			assert( var );

			if( var->type == TYPE_CALLABLE )
			{
				BAT_RETURN( *var );
			}

			BAT_RETURN( var->GetField( node->Attr().lexeme ) );
		}

		BAT_RETURN( BatObject() );
	}
	void Interpreter::VisitCastExpr( CastExpr* node )
	{
		if( !node->Expr()->Type()->IsPrimitive() || !node->TargetType()->IsPrimitive() )
		{
			throw RuntimeError( node->Location(), "Cannot cast non-primitive types" );
		}

		PrimitiveType* base_type = node->Expr()->Type()->AsPrimitive();
		PrimitiveType* to_type = node->TargetType()->AsPrimitive();
		BatObject base = Evaluate( node->Expr() );
		switch( base_type->PrimKind() )
		{
		case PrimitiveKind::Void:
		{
			throw RuntimeError( node->Location(), "Cannot cast void type" );
		}
		case PrimitiveKind::Bool:
		{
			switch( to_type->PrimKind() )
			{
			case PrimitiveKind::Int:
				BAT_RETURN( BatObject::Int( (int64_t)base.GetBool() ) );
			case PrimitiveKind::Bool:
			case PrimitiveKind::Float:
			case PrimitiveKind::String:
			default:
				assert( false );
			}
		}
		case PrimitiveKind::Int:
		{
			switch( to_type->PrimKind() )
			{
			case PrimitiveKind::Bool:
				BAT_RETURN( BatObject::Bool( (bool)base.GetInt() ) );
			case PrimitiveKind::Float:
				BAT_RETURN( BatObject::Float( (float)base.GetInt() ) );
			case PrimitiveKind::String:
				BAT_RETURN( BatObject::String( std::to_string( base.GetInt() ).c_str() ) );
			default:
				assert( false );
			}
		}
		case PrimitiveKind::Float:
		{
			switch( to_type->PrimKind() )
			{
			case PrimitiveKind::Bool:
				BAT_RETURN( BatObject::Bool( (bool)base.GetFloat() ) );
			case PrimitiveKind::Int:
				BAT_RETURN( BatObject::Int( (int64_t)base.GetFloat() ) );
			case PrimitiveKind::String:
				BAT_RETURN( BatObject::String( std::to_string( base.GetFloat() ).c_str() ) );
			default:
				assert( false );
			}
		}
		case PrimitiveKind::String:
		{
			switch( to_type->PrimKind() )
			{
			case PrimitiveKind::Bool:
				BAT_RETURN( BatObject::True() );
			case PrimitiveKind::Int:
				BAT_RETURN( BatObject::Int( std::stoll( base.GetString() ) ) );
			case PrimitiveKind::Float:
				BAT_RETURN( BatObject::Float( std::stod( base.GetString() ) ) );
			default:
				assert( false );
			}
		}
		}
	}
	void Interpreter::VisitGroupExpr( GroupExpr* node )
	{
		BAT_RETURN( Evaluate( node->Expr() ) );
	}
	void Interpreter::VisitVarExpr( VarExpr* node )
	{
		BAT_RETURN( GetVar( node->Identifier().lexeme, node->Location() ) );
	}
	void Interpreter::VisitExpressionStmt( ExpressionStmt* node )
	{
		Evaluate( node->Expr() );
	}
	void Interpreter::VisitAssignStmt( AssignStmt* node )
	{
		try
		{
			Expression* l = node->Left();
			Expression* r = node->Right();
			if( !l->IsVarExpr() && !l->IsIndexExpr() )
			{
				throw RuntimeError( l->Location(), "Expression must be a modifiable lvalue" );
			}

			BatObject current;

			if( VarExpr* v = l->ToVarExpr() )
			{
				current = Evaluate( l );
			}
			else if( IndexExpr* i = l->ToIndexExpr() )
			{
				BatObject arr = Evaluate( i->Array() );
				BatObject index = Evaluate( i->Index() );
				current = arr.Index( index );
			}
			BatObject assign = Evaluate( r );

			BatObject newval;
			switch( node->Op() )
			{
				case TOKEN_EQUAL:          newval = assign; break;
				case TOKEN_PLUS_EQUAL:     newval = current.Add( assign ); break;
				case TOKEN_MINUS_EQUAL:    newval = current.Sub( assign ); break;
				case TOKEN_ASTERISK_EQUAL: newval = current.Mul( assign ); break;
				case TOKEN_SLASH_EQUAL:    newval = current.Div( assign ); break;
				case TOKEN_PERCENT_EQUAL:  newval = current.Mod( assign ); break;
				case TOKEN_AMP_EQUAL:      newval = current.BitAnd( assign ); break;
				case TOKEN_HAT_EQUAL:      newval = current.BitXor( assign ); break;
				case TOKEN_BAR_EQUAL:      newval = current.BitOr( assign ); break;
				default:
					throw RuntimeError( node->Location(), "Unexpected assign statement" );
			}
			if( VarExpr* v = l->ToVarExpr() )
			{
				SetVar( v->Identifier().lexeme, newval, node->Location() );
			}
			else if( IndexExpr* i = l->ToIndexExpr() )
			{
				BatObject arr = Evaluate( i->Array() );
				BatObject index = Evaluate( i->Index() );
				arr.Index( index ) = newval;
			}
			BAT_RETURN( newval );
		}
		catch( const BatObjectError& e )
		{
			throw RuntimeError( node->Location(), e.what() );
		}
	}
	void Interpreter::VisitBlockStmt( BlockStmt* node )
	{
		Environment environment( m_pEnvironment );
		size_t count = node->NumStatements();
		for( size_t i = 0; i < count; i++ )
		{
			ExecuteBlock( node->Stmt( i ), environment );
		}
	}
	void Interpreter::VisitPrintStmt( PrintStmt* node )
	{
		std::cout << Evaluate( node->Expr() ).ToString() << std::endl;
	}
	void Interpreter::VisitIfStmt( IfStmt* node )
	{
		BatObject condition = Evaluate( node->Condition() );
		if( IsTruthy( condition, node->Condition()->Location() ) )
		{
			Execute( node->Then() );
		}
		else if( node->Else() )
		{
			Execute( node->Else() );
		}
	}
	void Interpreter::VisitWhileStmt( WhileStmt* node )
	{
		while( IsTruthy( Evaluate( node->Condition() ), node->Condition()->Location() ) )
		{
			try
			{
				Execute( node->Body() );
			}
			catch ( const BreakLoop& )
			{
				break;
			}
			catch ( const ContinueLoop& )
			{
				continue;
			}
		}
	}
	void Interpreter::VisitForStmt( ForStmt* node )
	{
		for( Evaluate( node->Initializer() );
			IsTruthy( Evaluate( node->Condition() ), node->Condition()->Location() );
			Evaluate( node->Increment() ) )
		{
			try
			{
				Execute( node->Body() );
			}
			catch ( const BreakLoop& )
			{
				break;
			}
			catch ( const ContinueLoop& )
			{
				continue;
			}
		}
	}
	void Interpreter::VisitReturnStmt( ReturnStmt* node )
	{
		ReturnValue ret;
		if( node->RetExpr() )
		{
			ret.value = Evaluate( node->RetExpr() );
		}

		throw ret;
	}
	void Interpreter::VisitBreakStmt( BreakStmt* node )
	{
		throw BreakLoop();
	}
	void Interpreter::VisitContinueStmt( ContinueStmt* node )
	{
		throw ContinueLoop();
	}
	void Interpreter::VisitImportStmt( ImportStmt* node )
	{
		std::string filename = node->ModuleName() + ".bat";
		if( !std::ifstream( filename ) )
		{
			filename = node->ModuleName() + ".bs";
			if( !std::ifstream( filename ) )
			{
				ErrorSys::Report( node->Location().Line(), node->Location().Column(), "Module '" + node->ModuleName() + "' not found" );
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

		Environment* old_env = m_pEnvironment;
		m_pEnvironment = new Environment( node->ModuleName() );

		for( size_t i = 0; i < res.size(); i++ )
		{
			Execute( std::move( res[i] ) );
		}

		Environment* module_env = m_pEnvironment;
		m_pEnvironment = old_env;

		if( node->HasImports() )
		{
			const auto& imports = node->Imports();
			if( imports[0].type == TOKEN_ASTERISK )
			{
				m_pEnvironment->StealVarsFrom( *module_env );
			}
			else
			{
				for( const Token& import_name : imports )
				{
					const BatObject* var = module_env->GetVar( import_name.lexeme );
					assert( var );

					m_pEnvironment->AddVar( import_name.lexeme, *var );
				}
			}

			delete module_env;
		}
		else
		{
			auto module = BatObject::Module( module_env );
			m_pEnvironment->AddVar( node->AsName(), module );
		}
	}
	void Interpreter::VisitNativeStmt( NativeStmt* node )
	{
		const BatModule* module = FindModule( m_pEnvironment->Module() );
		if( !module )
		{
			throw RuntimeError( node->Location(), "Unresolved native (no registered module)" );
		}
		
		const std::string& name = node->Signature().Identifier().lexeme;
		std::string native_resolve_name = (!m_pEnvironment->Class().empty()) ? (m_pEnvironment->Class() + "." + name) : name;

		const BatNativeCallback* callback = module->FindNative( native_resolve_name );
		if( !callback )
		{
			throw RuntimeError( node->Location(), "Unresolved native" );
		}

		auto native = BatObject::Function( new BatNative( *callback ) );
		AddVar( name, native, node->Location() );
	}
	void Interpreter::VisitVarDecl( VarDecl* node )
	{
		BatObject initial;
		if( node->Initializer() )
		{
			initial = Evaluate( node->Initializer() );
		}
		AddVar( node->Identifier().lexeme, initial, node->Identifier().loc );
	}
	void Interpreter::VisitFuncDecl( FuncDecl* node )
	{
		AddVar( node->Signature().Identifier().lexeme,
			BatObject::Function( new BatFunction( node ) ),
			node->Signature().Identifier().loc );
	}
	void Interpreter::VisitClassDecl( ClassDecl* node )
	{
		Environment* old_env = m_pEnvironment;
		Environment* class_env = new Environment( node->Name().lexeme, old_env );

		AddVar( node->Name().lexeme, BatObject::Class( class_env ), node->Name().loc );

		m_pEnvironment = class_env;

		for( auto& native : node->Natives() )
			Execute( native.get() );
		for( auto& method : node->Methods() )
			Execute( method.get() );
		for( auto& field : node->Fields() )
			Execute( field.get() );

		m_pEnvironment = old_env;
	}
}
