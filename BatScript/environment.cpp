#include "environment.h"

#include <Windows.h>

namespace Bat
{
	Environment::Environment( Environment* enclosing )
	{
		if( enclosing->m_pEnclosing == enclosing )
		{
			DebugBreak();
		}
		m_pEnclosing = enclosing;
	}

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

	void Environment::AddVar( const std::string& name, const BatObject& value )
	{
		if( ExistsLocally( name ) )
		{
			throw BatObjectError();
		}

		m_mapVariables[name] = value;
	}

	const BatObject& Environment::GetVar( const std::string& name ) const
	{
		auto it = m_mapVariables.find( name );
		if( it == m_mapVariables.end() )
		{
			if( m_pEnclosing )
			{
				return m_pEnclosing->GetVar( name );
			}
			throw BatObjectError();
		}
		return it->second;
	}

	void Environment::SetVar( const std::string& name, const BatObject& value )
	{
		auto it = m_mapVariables.find( name );
		if( it == m_mapVariables.end() )
		{
			if( m_pEnclosing )
			{
				m_pEnclosing->SetVar( name, value );
			}
			throw BatObjectError();
		}
		m_mapVariables[name] = value;
	}
}
