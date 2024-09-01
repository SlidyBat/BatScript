#include "environment.h"

namespace Bat
{
	Environment::Environment( const std::string& module ) :
		m_Module( module )
	{}
    Environment::Environment(const std::string &classname, Environment *enclosing) :
		m_pEnclosing(enclosing)
    {
		m_Module = m_pEnclosing->Module();
		m_Class = classname;
    }
    Environment::Environment(Environment *enclosing) :
		m_pEnclosing(enclosing)
    {
		m_Module = m_pEnclosing->Module();
		m_Class = m_pEnclosing->Class();
	}

	const std::string& Environment::Module() const
	{
		return m_Module;
	}

	const std::string& Environment::Class() const
	{
		return m_Class;
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

	bool Environment::AddVar( const std::string& name, BatObject value )
	{
		if( ExistsLocally( name ) )
		{
			return false;
		}

		if( value.Type() == TYPE_CLASS )
		{
			value.Value().env->m_pEnclosing = this;
		}

		m_mapVariables[name] = std::move( value );
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

	bool Environment::StealVarsFrom( Environment& from )
	{
		for( auto& it : from.m_mapVariables )
		{
			if( it.second.Type() == TYPE_MODULE )
				continue;
			
			if( !AddVar( it.first, std::move( it.second ) ) )
				return false;
		}

		if( from.m_pEnclosing )
			return StealVarsFrom( *from.m_pEnclosing );

		return true;
	}
}
