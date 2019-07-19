#include "bat_callable.h"

#include "interpreter.h"

namespace Bat
{
	BatFunction::BatFunction( FuncDecl* declaration )
		:
		m_pDeclaration( declaration )
	{
		size_t i;
		for( i = 0; i < m_pDeclaration->NumParams(); i++ )
		{
			if( m_pDeclaration->Default( i ) != nullptr )
			{ 
				break;
			}
		}
		m_nDefaults = m_pDeclaration->NumParams() - i;
	}
	size_t BatFunction::Arity() const
	{
		return m_pDeclaration->NumParams();
	}
	size_t BatFunction::NumDefaults() const
	{
		return m_nDefaults;
	}
	BatObject BatFunction::Call( Interpreter& interpreter, const std::vector<BatObject>& args )
	{
		Environment environment( interpreter.GetEnvironment() );
		for( size_t i = 0; i < args.size(); i++ )
		{
			environment.AddVar( m_pDeclaration->Param( i ).lexeme, args[i], m_pDeclaration->Param( i ).loc );
		}
		for( size_t i = args.size(); i < m_pDeclaration->NumParams(); i++ )
		{
			environment.AddVar( m_pDeclaration->Param( i ).lexeme,
				interpreter.Evaluate( m_pDeclaration->Default( i ) ),
				m_pDeclaration->Param( i ).loc );
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

	BatNative::BatNative( size_t arity, BatNativeCallback callback )
		:
		m_iArity( arity ),
		m_Callback( std::move( callback ) )
	{}
	size_t BatNative::Arity() const
	{
		return m_iArity;
	}
	BatObject BatNative::Call( Interpreter& interpreter, const std::vector<BatObject>& args )
	{
		return m_Callback( args );
	}
}
