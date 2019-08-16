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
		m_pEnvironment = new Environment;
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

	void Interpreter::AddNative( const std::string& name, BatNativeCallback callback )
	{
		auto native = BatObject( new BatNative( std::move( callback ) ) );
		auto loc = SourceLoc( 0, 0 );
		AddVar( name, native, loc );
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

	void Interpreter::VisitIntLiteral( IntLiteral* node )
	{
		BAT_RETURN( node->value );
	}
	void Interpreter::VisitFloatLiteral( FloatLiteral* node )
	{
		BAT_RETURN( node->value );
	}
	void Interpreter::VisitStringLiteral( StringLiteral* node )
	{
		BAT_RETURN( node->value );
	}
	void Interpreter::VisitTokenLiteral( TokenLiteral* node )
	{
		switch( node->value )
		{
			case TOKEN_TRUE:  BAT_RETURN( true );
			case TOKEN_FALSE: BAT_RETURN( false );
			case TOKEN_NIL:   BAT_RETURN( BatObject() );
		}
		throw RuntimeError( node->Location(), std::string( "Unexpected token '" ) + TokenTypeToString( node->value ) + "'" );
	}
	void Interpreter::VisitArrayLiteral( ArrayLiteral* node )
	{
		auto arr = std::make_unique<BatObject[]>( node->NumValues() );
		for( size_t i = 0; i < node->NumValues(); i++ )
		{
			arr[i] = Evaluate( node->ValueAt( i ) );
		}

		BatObject obj = BatObject( arr.get(), node->NumValues(), ArrayType::UNSIZED );

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
				if( !l->IsVarExpr() && !l->IsIndexExpr() )
				{
					throw RuntimeError( l->Location(), "Expression must be a modifiable lvalue" );
				}

				BatObject current;

				if( VarExpr * v = l->ToVarExpr() )
				{
					current = Evaluate( l );
				}
				else if( IndexExpr * i = l->ToIndexExpr() )
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
				}
				if( VarExpr* v = l->ToVarExpr() )
				{
					SetVar( v->name.lexeme, newval, node->Location() );
				}
				else if( IndexExpr* i = l->ToIndexExpr() )
				{
					BatObject arr = Evaluate( i->Array() );
					BatObject index = Evaluate( i->Index() );
					arr.Index( index ) = newval;
				}
				BAT_RETURN( newval );
			}

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
				if( Evaluate( l ).IsTruthy() ) BAT_RETURN( true );
				if( Evaluate( r ).IsTruthy() ) BAT_RETURN( true );
				BAT_RETURN( false );
			case TOKEN_AND:
				if( !Evaluate( l ).IsTruthy() ) BAT_RETURN( false );
				if( !Evaluate( r ).IsTruthy() ) BAT_RETURN( false );
				BAT_RETURN( true );
			}
		}
		catch( const BatObjectError& e )
		{
			throw RuntimeError( node->Location(), e.what() );
		}

		throw RuntimeError( node->Location(), "Unexpected binary expression" );
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
			}
		}
		catch( const BatObjectError& e )
		{
			throw RuntimeError( node->Location(), e.what() );
		}

		throw RuntimeError( node->Location(), "Unexpected binary expression" );
	}
	void Interpreter::VisitCallExpr( CallExpr* node )
	{
		try
		{
			BatObject func = Evaluate( node->Function() );
			size_t num_args = node->NumArgs();
			std::vector<BatObject> arguments;
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
	void Interpreter::VisitGroupExpr( GroupExpr* node )
	{
		BAT_RETURN( Evaluate( node->Expr() ) );
	}
	void Interpreter::VisitVarExpr( VarExpr* node )
	{
		BAT_RETURN( GetVar( node->name.lexeme, node->Location() ) );
	}
	void Interpreter::VisitExpressionStmt( ExpressionStmt* node )
	{
		Evaluate( node->Expr() );
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
			Execute( node->Body() );
		}
	}
	void Interpreter::VisitForStmt( ForStmt* node )
	{
		for( Evaluate( node->Initializer() );
			IsTruthy( Evaluate( node->Condition() ), node->Condition()->Location() );
			Evaluate( node->Increment() ) )
		{
			Execute( node->Body() );
		}
	}
	void Interpreter::VisitReturnStmt( ReturnStmt* node )
	{
		auto ret_value = Evaluate( node->RetValue() );
		ReturnValue ret;
		ret.value = ret_value;
		throw ret;
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
			Execute( std::move( res[i] ) );
		}
	}
	void Interpreter::VisitNativeStmt( NativeStmt* node )
	{
		// Natives are more like forward declarations as a hint to the compiler for static checks
		// When executing they aren't needed
	}
	void Interpreter::VisitVarDecl( VarDecl* node )
	{
		VarDecl* curr = node;
		while( curr )
		{
			BatObject initial;
			if( curr->Initializer() )
			{
				initial = Evaluate( curr->Initializer() );
			}
			AddVar( curr->Identifier().lexeme, initial, curr->Identifier().loc );
			
			curr = curr->Next();
		}
	}
	void Interpreter::VisitFuncDecl( FuncDecl* node )
	{
		AddVar( node->Signature().Identifier().lexeme, new BatFunction( node ), node->Signature().Identifier().loc );
	}
}
