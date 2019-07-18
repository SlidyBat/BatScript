#include "interpreter.h"

#include <iostream>
#include "bat_callable.h"

#define BAT_RETURN( value ) do { m_Result = (value); return; } while( false )

namespace Bat
{
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
	}

	void Interpreter::Execute( Statement* s )
	{
		s->Accept( this );
	}

	void Interpreter::ExecuteBlock( Statement* s, Environment& environment )
	{
		Environment* previous = m_pEnvironment;
		m_pEnvironment = &environment;

		Execute( s );

		m_pEnvironment = previous;
	}

	void Interpreter::AddNative( const std::string& name, BatCallable* native )
	{
		m_pEnvironment->AddVar( name, BatObject( native ) );
	}

	void Interpreter::VisitIntLiteral( Bat::IntLiteral* node )
	{
		BAT_RETURN( node->value );
	}
	void Interpreter::VisitFloatLiteral( Bat::FloatLiteral* node )
	{
		BAT_RETURN( node->value );
	}
	void Interpreter::VisitStringLiteral( Bat::StringLiteral* node )
	{
		BAT_RETURN( node->value );
	}
	void Interpreter::VisitTokenLiteral( Bat::TokenLiteral* node )
	{
		switch( node->value )
		{
			case TOKEN_TRUE:  BAT_RETURN( true );
			case TOKEN_FALSE: BAT_RETURN( false );
			case TOKEN_NIL:   BAT_RETURN( BatObject() );
		}
		throw BatObjectError();
	}
	void Interpreter::VisitBinaryExpr( Bat::BinaryExpr* node )
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
					throw BatObjectError();
				}
				std::string name = l->AsVarExpr()->name.lexeme;
				BatObject current = Evaluate( l );
				BatObject assign = Evaluate( r );

				BatObject newval;
				switch( node->Op() )
				{
					case TOKEN_EQUAL:          newval = assign; break;
					case TOKEN_PLUS_EQUAL:     newval = current + assign; break;
					case TOKEN_MINUS_EQUAL:    newval = current - assign; break;
					case TOKEN_ASTERISK_EQUAL: newval = current * assign; break;
					case TOKEN_SLASH_EQUAL:    newval = current / assign; break;
					case TOKEN_PERCENT_EQUAL:  newval = current % assign; break;
					case TOKEN_AMP_EQUAL:      newval = current & assign; break;
					case TOKEN_HAT_EQUAL:      newval = current ^ assign; break;
					case TOKEN_BAR_EQUAL:      newval = current | assign; break;
				}
				m_pEnvironment->SetVar( name, newval );
				BAT_RETURN( newval );
			}

			case TOKEN_BAR:              BAT_RETURN( Evaluate( l ) | Evaluate( r ) );
			case TOKEN_HAT:              BAT_RETURN( Evaluate( l ) ^ Evaluate( r ) );
			case TOKEN_AMP:              BAT_RETURN( Evaluate( l ) & Evaluate( r ) );
			case TOKEN_EQUAL_EQUAL:      BAT_RETURN( Evaluate( l ) == Evaluate( r ) );
			case TOKEN_EXCLMARK_EQUAL:   BAT_RETURN( Evaluate( l ) != Evaluate( r ) );
			case TOKEN_LESS:             BAT_RETURN( Evaluate( l ) < Evaluate( r ) );
			case TOKEN_LESS_EQUAL:       BAT_RETURN( Evaluate( l ) <= Evaluate( r ) );
			case TOKEN_GREATER:          BAT_RETURN( Evaluate( l ) > Evaluate( r ) );
			case TOKEN_GREATER_EQUAL:    BAT_RETURN( Evaluate( l ) >= Evaluate( r ) );
			case TOKEN_LESS_LESS:        BAT_RETURN( Evaluate( l ) << Evaluate( r ) );
			case TOKEN_GREATER_GREATER:  BAT_RETURN( Evaluate( l ) >> Evaluate( r ) );
			case TOKEN_PLUS:             BAT_RETURN( Evaluate( l ) + Evaluate( r ) );
			case TOKEN_MINUS:            BAT_RETURN( Evaluate( l ) - Evaluate( r ) );
			case TOKEN_ASTERISK:         BAT_RETURN( Evaluate( l ) * Evaluate( r ) );
			case TOKEN_SLASH:            BAT_RETURN( Evaluate( l ) / Evaluate( r ) );
			case TOKEN_PERCENT:          BAT_RETURN( Evaluate( l ) % Evaluate( r ) );

			case TOKEN_OR:
				if( Evaluate( l ).IsTruthy() ) BAT_RETURN( true );
				if( Evaluate( r ).IsTruthy() ) BAT_RETURN( true );
				BAT_RETURN( false );
			case TOKEN_AND:
				if( !Evaluate( l ).IsTruthy() ) BAT_RETURN( false );
				if( !Evaluate( r ).IsTruthy() ) BAT_RETURN( false );
				BAT_RETURN( true );
		}
		throw BatObjectError();
	}
	void Interpreter::VisitUnaryExpr( Bat::UnaryExpr* node )
	{
		switch( node->Op() )
		{
			case TOKEN_MINUS:    BAT_RETURN( -Evaluate( node->Right() ) );
			case TOKEN_EXCLMARK: BAT_RETURN( !Evaluate( node->Right() ) );
			case TOKEN_TILDE:    BAT_RETURN( ~Evaluate( node->Right() ) );
				// case TOKEN_AMP:
		}
		throw BatObjectError();
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

		BAT_RETURN( func( *this, arguments ) );
	}
	void Interpreter::VisitGroupExpr( Bat::GroupExpr* node )
	{
		BAT_RETURN( Evaluate( node->Expr() ) );
	}
	void Interpreter::VisitVarExpr( Bat::VarExpr* node )
	{
		BAT_RETURN( m_pEnvironment->GetVar( node->name.lexeme ) );
	}
	void Interpreter::VisitExpressionStmt( Bat::ExpressionStmt* node )
	{
		Evaluate( node->Expr() );
	}
	void Interpreter::VisitBlockStmt( BlockStmt* node )
	{
		auto environment = Environment( m_pEnvironment );

		try
		{
			size_t count = node->NumStatements();
			for( size_t i = 0; i < count; i++ )
			{
				ExecuteBlock( node->Stmt( i ), environment );
			}
		}
		catch( const BatObjectError& )
		{
		}
	}
	void Interpreter::VisitPrintStmt( Bat::PrintStmt* node )
	{
		std::cout << Evaluate( node->Expr() ).ToString() << std::endl;
	}
	void Interpreter::VisitIfStmt( IfStmt* node )
	{
		BatObject condition = Evaluate( node->Condition() );
		if( condition.IsTruthy() )
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
		while( Evaluate( node->Condition() ).IsTruthy() )
		{
			Execute( node->Body() );
		}
	}
	void Interpreter::VisitForStmt( ForStmt* node )
	{
		for( Evaluate( node->Initializer() ); Evaluate( node->Condition() ).IsTruthy(); Evaluate( node->Increment() ) )
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
	void Interpreter::VisitVarDecl( Bat::VarDecl* node )
	{
		BatObject initial;
		if( node->Initializer() )
		{
			initial = Evaluate( node->Initializer() );
		}
		m_pEnvironment->AddVar( node->Identifier().lexeme, initial );
	}
	void Interpreter::VisitFuncDecl( FuncDecl* node )
	{
		m_pEnvironment->AddVar( node->Identifier().lexeme, new BatFunction( node ) );
	}
}
