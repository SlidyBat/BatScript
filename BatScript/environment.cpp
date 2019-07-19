#include "environment.h"

#include "runtime_error.h"

namespace Bat
{
	Environment::Environment( Environment* enclosing )
		:
		m_pEnclosing( enclosing )
	{}

	bool Environment::Exists( const std::string& name ) const
	{
		auto it = m_mapVariables.find( name );
		if( it == m_mapVariables.end() )
		{
			if( m_pEnclosing )
			{
				return m_pEnclosing->Exists( name );
			}
			return false;
		}
		return true;
	}

	bool Environment::ExistsLocally( const std::string& name ) const
	{
		auto it = m_mapVariables.find( name );
		if( it != m_mapVariables.end() )
		{
			return true;
		}

		return false;
	}

	void Environment::AddVar( const std::string& name, const BatObject& value, const SourceLoc& loc )
	{
		if( ExistsLocally( name ) )
		{
			throw RuntimeError( loc, name + " already exists in this scope" );
		}

		m_mapVariables[name] = value;
	}

	const BatObject& Environment::GetVar( const std::string& name, const SourceLoc& loc ) const
	{
		auto it = m_mapVariables.find( name );
		if( it == m_mapVariables.end() )
		{
			if( m_pEnclosing )
			{
				return m_pEnclosing->GetVar( name, loc );
			}
			throw RuntimeError( loc, name + " is not defined" );
		}
		return it->second;
	}

	void Environment::SetVar( const std::string& name, const BatObject& value, const SourceLoc& loc )
	{
		auto it = m_mapVariables.find( name );
		if( it == m_mapVariables.end() )
		{
			if( m_pEnclosing )
			{
				m_pEnclosing->SetVar( name, value, loc );
			}
			throw RuntimeError( loc, name + " is not defined" );
		}
		m_mapVariables[name] = value;
	}
}
