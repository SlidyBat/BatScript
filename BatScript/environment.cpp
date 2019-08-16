#include "environment.h"

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

	bool Environment::AddVar( const std::string& name, const BatObject& value )
	{
		if( ExistsLocally( name ) )
		{
			return false;
		}

		m_mapVariables[name] = value;
		return true;
	}

	const BatObject* Environment::GetVar( const std::string& name ) const
	{
		auto it = m_mapVariables.find( name );
		if( it == m_mapVariables.end() )
		{
			if( m_pEnclosing )
			{
				return m_pEnclosing->GetVar( name );
			}
			return nullptr;
		}
		return &it->second;
	}

	bool Environment::SetVar( const std::string& name, const BatObject& value )
	{
		auto it = m_mapVariables.find( name );
		if( it == m_mapVariables.end() )
		{
			if( m_pEnclosing )
			{
				m_pEnclosing->SetVar( name, value );
			}
			else
			{
				return false;
			}
		}
		m_mapVariables[name] = value;
		return true;
	}
}
