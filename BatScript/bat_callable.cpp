#include "bat_callable.h"

#include "interpreter.h"
#include "runtime_error.h"

namespace Bat
{
	static void AddVar( Environment& env, const Token& tok, const BatObject& value );

	BatFunction::BatFunction( FuncDecl* declaration )
		:
		m_pDeclaration( declaration )
	{
		const auto& sig = m_pDeclaration->Signature();

		size_t i;
		for( i = 0; i < sig.NumParams(); i++ )
		{
			if( sig.ParamDefault( i ) != nullptr )
			{ 
				break;
			}
		}
		m_nDefaults = sig.NumParams() - i;
	}
	size_t BatFunction::NumDefaults() const
	{
		return m_nDefaults;
	}
	BatObject BatFunction::Call( Interpreter& interpreter, const std::vector<BatObject>& args )
	{
		const auto& sig = m_pDeclaration->Signature();

		Environment environment( interpreter.GetEnvironment() );
		for( size_t i = 0; i < args.size(); i++ )
		{
			AddVar( environment, sig.ParamIdent( i ), args[i] );
		}
		for( size_t i = args.size(); i < m_pDeclaration->Signature().NumParams(); i++ )
		{
			AddVar( environment, sig.ParamIdent( i ), interpreter.Evaluate( sig.ParamDefault( i ) ) );
		}

		try
		{
			interpreter.ExecuteBlock( m_pDeclaration->Body(), environment );
		}
		catch( const ReturnValue& ret )
		{
			return ret.value;
		}

		return BatObject();
	}

	static void AddVar( Environment& env, const Token& tok, const BatObject& value )
	{
		if( !env.AddVar( tok.lexeme, value ) )
		{
			throw RuntimeError( tok.loc, tok.lexeme + " is already defined in this scope" );
		}
	}

	BatNative::BatNative( BatNativeCallback callback )
		:
		m_Callback( std::move( callback ) )
	{}
	BatObject BatNative::Call( Interpreter& interpreter, const std::vector<BatObject>& args )
	{
		return m_Callback( args );
	}
}
