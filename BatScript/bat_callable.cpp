#include "bat_callable.h"

#include "interpreter.h"

namespace Bat
{
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
			environment.AddVar( sig.ParamIdent( i ).lexeme, args[i], sig.ParamIdent( i ).loc );
		}
		for( size_t i = args.size(); i < m_pDeclaration->Signature().NumParams(); i++ )
		{
			environment.AddVar( sig.ParamIdent( i ).lexeme,
				interpreter.Evaluate( sig.ParamDefault( i ) ),
				sig.ParamIdent( i ).loc );
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

	BatNative::BatNative( BatNativeCallback callback )
		:
		m_Callback( std::move( callback ) )
	{}
	BatObject BatNative::Call( Interpreter& interpreter, const std::vector<BatObject>& args )
	{
		return m_Callback( args );
	}
}
