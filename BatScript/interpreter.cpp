#include "interpreter.h"

#include <iostream>
#include "bat_callable.h"
#include "runtime_error.h"

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

	void Interpreter::AddNative( const std::string& name, size_t arity, BatNativeCallback callback )
	{
		m_pEnvironment->AddVar( name, BatObject( new BatNative( arity, std::move( callback ) ) ), SourceLoc( 0, 0 ) );
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
	void Interpreter::VisitBinaryExpr( BinaryExpr* node )
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
				if( !l->IsVarExpr() )
				{
					throw RuntimeError( l->Location(), "Tried to assign to a non-lvalue" );
				}
				std::string name = l->AsVarExpr()->name.lexeme;
				BatObject current = Evaluate( l );
				BatObject assign = Evaluate( r );

				BatObject newval;
				switch( node->Op() )
				{
					case TOKEN_EQUAL:          newval = assign; break;
					case TOKEN_PLUS_EQUAL:     newval = current.Add( assign, node->Location() ); break;
					case TOKEN_MINUS_EQUAL:    newval = current.Sub( assign, node->Location() ); break;
					case TOKEN_ASTERISK_EQUAL: newval = current.Mul( assign, node->Location() ); break;
					case TOKEN_SLASH_EQUAL:    newval = current.Div( assign, node->Location() ); break;
					case TOKEN_PERCENT_EQUAL:  newval = current.Mod( assign, node->Location() ); break;
					case TOKEN_AMP_EQUAL:      newval = current.BitAnd( assign, node->Location() ); break;
					case TOKEN_HAT_EQUAL:      newval = current.BitXor( assign, node->Location() ); break;
					case TOKEN_BAR_EQUAL:      newval = current.BitOr( assign, node->Location() ); break;
				}
				m_pEnvironment->SetVar( name, newval, node->Location() );
				BAT_RETURN( newval );
			}

			case TOKEN_BAR:              BAT_RETURN( Evaluate( l ).BitOr( Evaluate( r ), node->Location() ) );
			case TOKEN_HAT:              BAT_RETURN( Evaluate( l ).BitXor( Evaluate( r ), node->Location() ) );
			case TOKEN_AMP:              BAT_RETURN( Evaluate( l ).BitAnd( Evaluate( r ), node->Location() ) );
			case TOKEN_EQUAL_EQUAL:      BAT_RETURN( Evaluate( l ).CmpEq( Evaluate( r ), node->Location() ) );
			case TOKEN_EXCLMARK_EQUAL:   BAT_RETURN( Evaluate( l ).CmpNeq( Evaluate( r ), node->Location() ) );
			case TOKEN_LESS:             BAT_RETURN( Evaluate( l ).CmpL( Evaluate( r ), node->Location() ) );
			case TOKEN_LESS_EQUAL:       BAT_RETURN( Evaluate( l ).CmpLe( Evaluate( r ), node->Location() ) );
			case TOKEN_GREATER:          BAT_RETURN( Evaluate( l ).CmpG( Evaluate( r ), node->Location() ) );
			case TOKEN_GREATER_EQUAL:    BAT_RETURN( Evaluate( l ).CmpGe( Evaluate( r ), node->Location() ) );
			case TOKEN_LESS_LESS:        BAT_RETURN( Evaluate( l ).LShift( Evaluate( r ), node->Location() ) );
			case TOKEN_GREATER_GREATER:  BAT_RETURN( Evaluate( l ).RShift( Evaluate( r ), node->Location() ) );
			case TOKEN_PLUS:             BAT_RETURN( Evaluate( l ).Add( Evaluate( r ), node->Location() ) );
			case TOKEN_MINUS:            BAT_RETURN( Evaluate( l ).Sub( Evaluate( r ), node->Location() ) );
			case TOKEN_ASTERISK:         BAT_RETURN( Evaluate( l ).Mul( Evaluate( r ), node->Location() ) );
			case TOKEN_SLASH:            BAT_RETURN( Evaluate( l ).Div( Evaluate( r ), node->Location() ) );
			case TOKEN_PERCENT:          BAT_RETURN( Evaluate( l ).Mod( Evaluate( r ), node->Location() ) );

			case TOKEN_OR:
				if( Evaluate( l ).IsTruthy( l->Location() ) ) BAT_RETURN( true );
				if( Evaluate( r ).IsTruthy( r->Location() ) ) BAT_RETURN( true );
				BAT_RETURN( false );
			case TOKEN_AND:
				if( !Evaluate( l ).IsTruthy( l->Location() ) ) BAT_RETURN( false );
				if( !Evaluate( r ).IsTruthy( r->Location() ) ) BAT_RETURN( false );
				BAT_RETURN( true );
		}
		throw RuntimeError( node->Location(), "Unexpected binary expression" );
	}
	void Interpreter::VisitUnaryExpr( UnaryExpr* node )
	{
		switch( node->Op() )
		{
			case TOKEN_MINUS:    BAT_RETURN( Evaluate( node->Right() ).Neg( node->Location() ) );
			case TOKEN_EXCLMARK: BAT_RETURN( Evaluate( node->Right() ).Not( node->Location() ) );
			case TOKEN_TILDE:    BAT_RETURN( Evaluate( node->Right() ).BitNeg( node->Location() ) );
				// case TOKEN_AMP:
		}
		throw RuntimeError( node->Location(), "Unexpected binary expression" );
	}
	void Interpreter::VisitCallExpr( CallExpr* node )
	{
		BatObject func = Evaluate( node->Function() );
		size_t num_args = node->NumArgs();
		std::vector<BatObject> arguments;
		for( size_t i = 0; i < num_args; i++ )
		{
			arguments.push_back( Evaluate( node->Arg( i ) ) );
		}

		BAT_RETURN( func.Call( *this, arguments, node->Location() ) );
	}
	void Interpreter::VisitGroupExpr( GroupExpr* node )
	{
		BAT_RETURN( Evaluate( node->Expr() ) );
	}
	void Interpreter::VisitVarExpr( VarExpr* node )
	{
		BAT_RETURN( m_pEnvironment->GetVar( node->name.lexeme, node->Location() ) );
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
		if( condition.IsTruthy( node->Condition()->Location() ) )
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
		while( Evaluate( node->Condition() ).IsTruthy( node->Condition()->Location() ) )
		{
			Execute( node->Body() );
		}
	}
	void Interpreter::VisitForStmt( ForStmt* node )
	{
		for( Evaluate( node->Initializer() ); Evaluate( node->Condition() ).IsTruthy( node->Condition()->Location() ); Evaluate( node->Increment() ) )
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
			m_pEnvironment->AddVar( curr->Identifier().lexeme, initial, curr->Identifier().loc );
			
			curr = curr->Next();
		}
	}
	void Interpreter::VisitFuncDecl( FuncDecl* node )
	{
		m_pEnvironment->AddVar( node->Identifier().lexeme, new BatFunction( node ), node->Identifier().loc );
	}
}
